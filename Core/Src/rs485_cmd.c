#include "rs485_cmd.h"
#include "rs485.h"
#include "sensor_types.h"
#include "calibration.h"
#include "flash_storage.h"
#include "filter.h"
#include <string.h>
#include "tim.h"
#include "usart.h"
#include "at24c04.h"

extern uint8_t ether_flag;

/* ================================================================
 *  Internal helpers
 * ================================================================ */

static void float_to_le(uint8_t *dst, float f)
{
    uint32_t u; memcpy(&u, &f, 4);
    dst[0] = (uint8_t)(u);
    dst[1] = (uint8_t)(u >> 8);
    dst[2] = (uint8_t)(u >> 16);
    dst[3] = (uint8_t)(u >> 24);
}

static float le_to_float(const uint8_t *src)
{
    uint32_t u = ((uint32_t)src[3] << 24)
               | ((uint32_t)src[2] << 16)
               | ((uint32_t)src[1] << 8)
               | ((uint32_t)src[0]);
    float f; memcpy(&f, &u, 4); return f;
}

/* Send a response frame: AA 55 [cmd|0x80] [data...] 0D 0A */
static void respond(uint8_t cmd, const uint8_t *data, uint16_t len)
{
    rs485_send_frame((uint8_t)(cmd | RS485_RESP_FLAG), data, len);
}

/* Send a single-byte ACK response */
static void ack(uint8_t cmd) { respond(cmd, NULL, 0); }

/* Map a baud-rate code (1..5) to the actual baud rate.
 * Returns 0 when the code is not one of the supported values. */
static uint32_t baud_from_code(uint8_t code)
{
    switch (code) {
    case RS485_BAUD_460800: return 460800U;
    case RS485_BAUD_256000: return 256000U;
    case RS485_BAUD_115200: return 115200U;
    case RS485_BAUD_19200:  return 19200U;
    case RS485_BAUD_9600:   return 9600U;
    default:                return 0U;
    }
}

/* Minimal float → ASCII: "-ddd.dddddd" (6 decimals), NUL-terminated.
   Written by hand because the firmware links with newlib-nano without
   -u _printf_float, so snprintf("%f") cannot print floats at all. */
static int fmt_float(char *dst, int max, float v)
{
    static const uint32_t pow10[6] = { 1UL, 10UL, 100UL, 1000UL, 10000UL, 100000UL };
    uint32_t ip, fp;
    char tmp[12];
    int n = 0, k = 0, i;

    if (max < 2) return 0;

    if (v != v) {                                  /* NaN guard */
        static const char nan_txt[] = "NaN";
        for (i = 0; nan_txt[i] != '\0' && n < max - 1; i++) {
            dst[n++] = nan_txt[i];
        }
        dst[n] = '\0';
        return n;
    }

    if (v < 0.0f) {
        if (n < max - 1) dst[n++] = '-';
        v = -v;
    }

    ip = (uint32_t)v;
    fp = (uint32_t)((v - (float)ip) * 1000000.0f + 0.5f);
    if (fp >= 1000000UL) { ip += 1UL; fp = 0UL; }  /* rounding carry */

    do {                                           /* integer part */
        tmp[k++] = (char)('0' + (ip % 10UL));
        ip /= 10UL;
    } while (ip != 0UL && k < (int)sizeof(tmp));
    while (k > 0 && n < max - 1) {
        dst[n++] = tmp[--k];
    }

    if (n < max - 1) dst[n++] = '.';

    for (i = 5; i >= 0; i--) {                     /* 6 decimals, zero-padded */
        if (n < max - 1) {
            dst[n++] = (char)('0' + ((fp / pow10[i]) % 10UL));
        }
    }

    dst[n] = '\0';
    return n;
}

/* Persist the data output unit (1 = kg, 2 = N) at DATA_FORMAT_ADDR so it
   survives a power cycle.  Only 0x33 and 0x35 store a value; any other
   content in that byte makes the firmware fall back to 2 (N) on boot. */
static void data_format_save(uint8_t fmt)
{
    uint8_t v = fmt;
    at24c04_write(DATA_FORMAT_ADDR, &v, 1);
}

/* Apply a new baud rate: save it to EEPROM, then re-init USART1 and restart
 * DMA reception.  Must be called AFTER the ACK has been transmitted so the
 * host can still read the response at the old baud rate. */
static void apply_baud(uint32_t baud)
{
    uint8_t  ee[4];
    uint16_t i;

    uart1_bsp = baud;

    /* Persist (little-endian), same layout as the legacy GM_MOD_BAUD command */
    for (i = 0; i < 4; i++) {
        ee[i] = (uint8_t)(baud >> (i * 8));
    }
    at24c04_write(UART_BAUD_ADDR, ee, 4);

    /* Wait until the last ACK byte has completely shifted out */
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET) {}
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);

    /* Stop DMA reception, then re-configure the UART at the new baud rate */
    if (huart1.hdmarx) {
        HAL_UART_DMAStop(&huart1);
    }
    HAL_UART_DeInit(&huart1);

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = baud;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);

    /* Re-enable IDLE-line detection and restart DMA reception */
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    rs485_rx_restart();
}

/* ================================================================
 *  rs485_send_data_frame
 * ================================================================ */
void rs485_send_data_frame(uint8_t cmd)
{
    uint8_t d[28];
    uint8_t i;

    /* debug_mode: 0 = framed output, 1 = raw floats + 00 00 80 7F,
       anything else = silent, nothing is sent at all. */
    if (g_sys.debug_mode != 0U && g_sys.debug_mode != 1U) {
        return;
    }

    for (i = 0; i < 6; i++) {
        float f = g_sensor.force[i];        /* read volatile once */
        float_to_le(d + i * 4, f);
    }

    if (g_sys.debug_mode == 1) {
        /* Debug mode (only 1): [6 × float32 LE] 00 00 80 7F */
        d[24] = 0x00;
        d[25] = 0x00;
        d[26] = 0x80;
        d[27] = 0x7F;
        just_float_send_raw(d, 28);
    } else {
        /* debug_mode == 0 — standard HEX frame:
           AA 55 [cmd] [6 × float32 LE] 0D 0A
           0x02 for the continuous stream, 0x03 for a single-shot reply.
           Shift the payload right by one to make room for the CMD byte. */
        for (i = 24; i >= 1; i--) {
            d[i] = d[i - 1];
        }
        d[0] = cmd;
        rs485_send_raw(d, 1 + 24);
    }
}

/* ================================================================
 *  rs485_cmd_dispatch
 * ================================================================ */
void rs485_cmd_dispatch(uint8_t cmd, const uint8_t *payload, uint16_t len)
{
    uint8_t buf[150];   /* enough for 144-byte matrix ack */

    switch (cmd) {

    /* ---- 0x01  Stop continuous send ---- */
    case RS485_CMD_STOP:
        g_sys.send_mode = 0;
        ack(cmd);
        break;

    /* ---- 0x02  Start continuous send ---- */
    case RS485_CMD_START:
        HAL_TIM_Base_Start_IT(&htim2);
        g_sys.send_mode = 1;
        ack(cmd);
        break;

    /* ---- 0x03  Single-shot request ---- */
    case RS485_CMD_SINGLE:
        /* AA 55 03 [6×float32] 0D 0A — same payload as the 0x02 stream,
           only the command byte differs so the host can tell them apart. */
        rs485_send_data_frame(RS485_CMD_SINGLE);
        break;

    /* ---- 0x04  Set baud rate: AA 55 04 [code] 0D 0A ---- */
    case RS485_CMD_SET_BAUD: {
        uint32_t baud;
        if (len < 1) { g_sys.comm_error_cnt++; break; }
        baud = baud_from_code(payload[0]);
        if (baud == 0U) { g_sys.comm_error_cnt++; break; }   /* invalid code */

        ack(cmd);               /* ACK first, at the OLD baud rate          */
        apply_baud(baud);       /* persist + re-init USART1 at the new rate */
        break;
    }

    /* ---- 0x08  Set frequency: AA 55 08 [0|1] 0D 0A ---- */
    case RS485_CMD_SET_FREQ:
        if (len < 1) { g_sys.comm_error_cnt++; break; }
        g_sys.freq_mode = (payload[0] == 0) ? 0U : 1U;
        ack(cmd);
        break;

    /* ---- 0x05  Query serial number ---- */
    case RS485_CMD_QUERY_SN:
        original_send((const uint8_t *)g_config.sn, (uint16_t)strlen(g_config.sn));
        break;

    /* ---- 0x06  Query firmware version ---- */
    case RS485_CMD_QUERY_VER:
        buf[0] = (uint8_t)(g_config.fw_version >> 16);
        buf[1] = (uint8_t)(g_config.fw_version >> 8);
        buf[2] = (uint8_t)(g_config.fw_version);
        respond(cmd, buf, 3);
        break;

    /* ---- 0x07  Query system status ---- */
    case RS485_CMD_QUERY_STATUS:
        buf[0] = g_sys.send_mode;
        buf[1] = g_sys.data_format;
        buf[2] = g_sys.freq_mode;
        buf[3] = g_sys.debug_mode;
        buf[4] = (uint8_t)(g_sys.status_flags);
        buf[5] = (uint8_t)(g_sys.status_flags >> 8);
        buf[6] = (uint8_t)(g_sys.comm_error_cnt);
        buf[7] = (uint8_t)(g_sys.comm_error_cnt >> 8);
        respond(cmd, buf, 8);
        break;

    /* ---- 0x30  Zero calibration (20-point force average) ---- */
    case RS485_CMD_ZERO:
        HAL_TIM_Base_Start_IT(&htim2);
        calib_zero_start();
        g_sys.zero_calib_busy = 1;
        ack(cmd);
        break;

    case RS485_CMD_HEX_SEND:
        g_sys.debug_mode = 0;
        ack(cmd);
        break;

    case RS485_CMD_ASC_SEND:
        g_sys.debug_mode = 1;
        ack(cmd);
        break;

    /* ---- 0x36  Cancel zero ---- */
    case RS485_CMD_UNZERO:
        calib_zero_cancel();
        // flash_save_zero();
        ack(cmd);
        break;


    /* ---- 0x37  Query 6×6 decoupling matrix, plain ASCII text ----
       One line per row, 6 lines total, no binary frame header:
           matrix1: 0.298418, -0.211604, ...\r\n
       Row order matches the EEPROM layout and the per-row upload of 0x20. */
    case RS485_CMD_QUERY_MATRIX: {
        static const char pfx[] = "matrix";
        char line[112];

        for (int r = 0; r < 6; r++) {
            int n = 0;
            int cap = (int)sizeof(line);

            /* "matrix1: " … "matrix6: " */
            memcpy(line, pfx, sizeof(pfx) - 1);
            n  = (int)(sizeof(pfx) - 1);
            line[n++] = (char)('1' + r);
            line[n++] = ':';
            line[n++] = ' ';

            for (int c = 0; c < 6; c++) {
                n += fmt_float(line + n, cap - n, g_matrix.m[r][c]);
                if (c != 5) {
                    if (n < cap - 1) line[n++] = ',';
                    if (n < cap - 1) line[n++] = ' ';
                }
            }
            if (n < cap - 2) {
                line[n++] = '\r';
                line[n++] = '\n';
            }
            /* Plain text, no AA 55 header — same style as the 0x05 SN query,
               so the output is readable as-is in a terminal. */
            original_send((const uint8_t *)line, (uint16_t)n);
        }
        break;
    }

    /* ---- 0x34  Set data format ---- */
    case RS485_CMD_SET_FORMAT:
        g_sys.data_format = 0;
        FloatFilter_Init();
        ack(cmd);
        break;

    /* ---- 0x33  Set data format: kg (persisted) ---- */
    case RS485_CMD_SET_FORMAT_KG:
        g_sys.data_format = 1;
        data_format_save(1);
        FloatFilter_Init();
        ack(cmd);
        break;

    /* ---- 0x35  Set data format: N (persisted) ---- */
    case RS485_CMD_SET_FORMAT_N:
        g_sys.data_format = 2;
        data_format_save(2);
        FloatFilter_Init();
        ack(cmd);
        break;

    /* ---- 0x20  Upload decoupling matrix (6×6 × 4 = 144 bytes) ---- */
    case RS485_CMD_SET_MATRIX:
        if (len < 25) { g_sys.comm_error_cnt++; break; }
        for (int i = 0; i < 6; i++) {
            g_matrix.m[payload[0]][i] = le_to_float(payload + 1 + i * 4);
        }
        flash_save_matrix(payload[0]);
        ack(cmd | payload[0]);
        break;

    /* ---- 0x35  Debug mode toggle ---- */
    // case RS485_CMD_DEBUG_MODE:
    //     g_sys.debug_mode = (uint8_t)(g_sys.debug_mode ? 0U : 1U);
    //     buf[0] = g_sys.debug_mode;
        // respond(cmd, buf, 1);
    //     break;

    default:
        /* Unknown command — silently ignored */
        g_sys.comm_error_cnt++;
        break;
    }
}
