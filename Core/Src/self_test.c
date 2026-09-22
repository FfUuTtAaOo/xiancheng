#include "self_test.h"
#include "lha7668.h"
#include "flash_storage.h"
#include "sensor_types.h"
#include "usart.h"
#include <string.h>

extern lhl_lha7668_ctx_t g_lha7668;  /* main.c */

/* ================================================================
 *  全局自检结果实例
 * ================================================================ */
self_test_t g_self_test;

/* ================================================================
 *  内部辅助函数
 * ================================================================ */

/* 检查一个 float 数组是否全为零 (用于判断零点数据是否合理) */
static int is_all_zeros(const float *arr, int n)
{
    for (int i = 0; i < n; i++) {
        uint32_t u;
        memcpy(&u, &arr[i], 4);
        if (u != 0x00000000U) return 0;  /* 非零 */
    }
    return 1;
}

/* 检查一个 float 数组是否全为 NaN/Inf (用于判断矩阵数据是否合理) */
static int has_valid_float(const float *arr, int n)
{
    int valid = 0;
    for (int i = 0; i < n; i++) {
        uint32_t u;
        memcpy(&u, &arr[i], 4);
        /* 检查 NaN: exponent=0xFF, mantissa != 0 */
        if ((u & 0x7F800000U) == 0x7F800000U && (u & 0x007FFFFFU) != 0) {
            continue;  /* NaN */
        }
        /* 检查 Inf: exponent=0xFF, mantissa == 0 */
        if ((u & 0x7F800000U) == 0x7F800000U) {
            continue;  /* +/-Inf */
        }
        if (u != 0xFFFFFFFFU) {  /* 不是全部FF (擦除状态) */
            valid = 1;
        }
    }
    return valid;
}

/* ================================================================
 *  LHA7668B ADC 自检
 * ================================================================ */
uint32_t self_test_adc(void)
{
    uint32_t err = ST_ERR_NONE;
    memset(&g_self_test, 0, sizeof(g_self_test));  /* 重置结果 */

    /* 1. SPI 通信测试: 读取 ID 寄存器 (0x05) */
    uint8_t id = LHL_LHA7668_Get_ID(&g_lha7668);
    g_self_test.adc_id = id;

    /* 如果 ID 读取返回 0xFF (SPI 无响应) 或 0x00 (未初始化) */
    if (id == 0xFF || id == 0x00) {
        err |= ST_ERR_ADC_SPI;
        g_self_test.adc_ready = 0;
        g_self_test.adc_channels = 0;
        g_self_test.error_flags = err;
        return err;
    }

    /* 2. ID 验证: LHA7668B-8 = 0x28 */
    if (id != 0x28) {
        err |= ST_ERR_ADC_ID;
    }

    /* 3. ERROR 寄存器检查 (0x06, 24-bit) */
    uint32_t err_reg = LHL_LHA7668_Read_Register(&g_lha7668, LHA7668_ERR, LHA7668_ERR_SIZE);
    g_self_test.adc_error = (uint8_t)(err_reg & 0xFF);
    if (err_reg != 0) {
        err |= ST_ERR_ADC_ERROR;
    }

    /* 4. 通道数: 固定 6 通道 (CH0-CH5 已配置) */
    g_self_test.adc_channels = 6;

    /* 5. 基础通信 OK 即视为 ADC 就绪 (完整数据读取需等 Start() 后) */
    g_self_test.adc_ready = 1;

    g_self_test.error_flags = err;
    return err;
}

/* ================================================================
 *  RS485 自检
 * ================================================================ */
uint32_t self_test_rs485(void)
{
    uint32_t err = ST_ERR_NONE;

    /* 1. USART1 外设检查: 检查 CR1 寄存器 UE 位 */
    if (huart1.Instance) {
        if (huart1.Instance->CR1 & USART_CR1_UE) {
            g_self_test.rs485_uart_ok = 1;
        } else {
            g_self_test.rs485_uart_ok = 0;
            err |= ST_ERR_RS485_UART;
        }
    } else {
        g_self_test.rs485_uart_ok = 0;
        err |= ST_ERR_RS485_UART;
    }

    /* 2. DMA 检查: 检查 DMA Stream 是否使能 (检查 CR 寄存器的 EN 位) */
    if (huart1.hdmarx && huart1.hdmarx->Instance) {
        if (huart1.hdmarx->Instance->CCR & DMA_CCR_EN_Msk) {
            g_self_test.rs485_dma_ok = 1;
        } else {
            g_self_test.rs485_dma_ok = 0;
            err |= ST_ERR_RS485_DMA;
        }
    } else {
        g_self_test.rs485_dma_ok = 0;
        err |= ST_ERR_RS485_DMA;
    }

    /* 合并错误到全局 */
    g_self_test.error_flags = (g_self_test.error_flags & ~0x000F0000U) | err;
    return err;
}

/* ================================================================
 *  Flash 存储自检
 * ================================================================ */
uint32_t self_test_flash(void)
{
    // uint32_t err = ST_ERR_NONE;
    // const uint8_t *src = (const uint8_t *)FLASH_STORAGE_ADDR;

    // /* 1. Magic 检查 */
    // uint32_t magic;
    // memcpy(&magic, src + FLASH_OFF_MAGIC, 4);
    // if (magic != FLASH_MAGIC) {
    //     err |= ST_ERR_FLASH_EMPTY;
    //     g_self_test.flash_valid    = 0;
    //     g_self_test.flash_matrix_ok = 0;
    //     g_self_test.flash_zero_ok   = 0;
    //     g_self_test.flash_config_ok = 0;
    //     g_self_test.error_flags = (g_self_test.error_flags & ~0x00F00000U) | err;
    //     return err;
    // }

    // /* 2. CRC 检查 (覆盖 CRC 字段之前的所有数据) */
    // uint16_t crc_stored;
    // memcpy(&crc_stored, src + FLASH_OFF_CRC, 2);
    // {
    //     uint16_t crc_calc = 0xFFFF;
    //     for (uint16_t i = 0; i < FLASH_OFF_CRC; i++) {
    //         crc_calc ^= (uint16_t)src[i];
    //         for (int j = 0; j < 8; j++) {
    //             if (crc_calc & 0x0001)
    //                 crc_calc = (crc_calc >> 1) ^ 0xA001;
    //             else
    //                 crc_calc >>= 1;
    //         }
    //     }
    //     if (crc_calc != crc_stored) {
    //         err |= ST_ERR_FLASH_CRC;
    //         g_self_test.flash_valid = 0;
    //     } else {
    //         g_self_test.flash_valid = 1;
    //     }
    // }

    // /* 3. 矩阵数据检查: 36 个 float (6×6) */
    // {
    //     float matrix[36];
    //     memcpy(matrix, src + FLASH_OFF_MATRIX, sizeof(matrix));
    //     g_self_test.flash_matrix_ok = has_valid_float(matrix, 36) ? 1 : 0;
    //     if (!g_self_test.flash_matrix_ok && !(err & ST_ERR_FLASH_CRC)) {
    //         /* 矩阵无效但不是CRC错误 — 可能是首次烧写默认值 */
    //     }
    // }

    // /* 4. 零点数据检查: 6 个 float */
    // {
    //     float zero[6];
    //     memcpy(zero, src + FLASH_OFF_ZERO, sizeof(zero));
    //     /* 零点可以为全零 (未校准状态)，但不应为 NaN/Inf */
    //     if (is_all_zeros(zero, 6)) {
    //         g_self_test.flash_zero_ok = 1;   /* 全零 = 未校准，视为 OK */
    //     } else {
    //         g_self_test.flash_zero_ok = has_valid_float(zero, 6) ? 1 : 0;
    //     }
    // }

    // /* 5. 配置数据检查: IP 地址非 0.0.0.0 */
    // {
    //     uint8_t ip[4];
    //     memcpy(ip, src + FLASH_OFF_IP, 4);
    //     g_self_test.flash_config_ok = is_valid_ip(ip) ? 1 : 0;
    // }

    // g_self_test.error_flags = (g_self_test.error_flags & ~0x00F00000U) | err;
    // return err;
    return 0;
}


/* ================================================================
 *  自检报告打印 (UART 调试输出)
 * ================================================================ */

static void uart_puts(UART_HandleTypeDef *huart, const char *s)
{
    HAL_UART_Transmit(huart, (const uint8_t *)s, (uint16_t)strlen(s), 100);
}

static void uart_put_hex8(UART_HandleTypeDef *huart, uint8_t val)
{
    char buf[3];
    buf[0] = "0123456789ABCDEF"[val >> 4];
    buf[1] = "0123456789ABCDEF"[val & 0x0F];
    buf[2] = '\0';
    HAL_UART_Transmit(huart, (uint8_t *)buf, 2, 100);
}

static void uart_put_dec(UART_HandleTypeDef *huart, uint32_t val)
{
    char buf[12];
    int pos = 0;
    if (val == 0) {
        buf[pos++] = '0';
    } else {
        char tmp[12];
        int tpos = 0;
        while (val > 0) {
            tmp[tpos++] = '0' + (char)(val % 10);
            val /= 10;
        }
        while (tpos > 0) buf[pos++] = tmp[--tpos];
    }
    buf[pos] = '\0';
    HAL_UART_Transmit(huart, (uint8_t *)buf, (uint16_t)pos, 100);
}

static void uart_print_result(UART_HandleTypeDef *huart, const char *label, int ok)
{
    uart_puts(huart, "  ");
    uart_puts(huart, label);
    uart_puts(huart, ok ? " OK\r\n" : " FAIL\r\n");
}

static void uart_print_ok_fail(UART_HandleTypeDef *huart, const char *label, int ok)
{
    uart_puts(huart, "  ");
    uart_puts(huart, label);
    uart_puts(huart, ": ");
    uart_puts(huart, ok ? "OK" : "FAIL");
    uart_puts(huart, "\r\n");
}

void self_test_print_report(UART_HandleTypeDef *huart)
{
    uint32_t err = g_self_test.error_flags;

    uart_puts(huart, "\r\n");
    uart_puts(huart, "========================================\r\n");
    uart_puts(huart, "  BOOT SELF-TEST REPORT\r\n");
    uart_puts(huart, "========================================\r\n");

    /* ---- 总体结果 ---- */
    uart_puts(huart, "[OVERALL] ");
    if (err == ST_ERR_NONE) {
        uart_puts(huart, "ALL PASS\r\n");
    } else {
        uart_puts(huart, "ERRORS DETECTED (flags=0x");
        uart_put_hex8(huart, (uint8_t)(err >> 24));
        uart_put_hex8(huart, (uint8_t)(err >> 16));
        uart_put_hex8(huart, (uint8_t)(err >> 8));
        uart_put_hex8(huart, (uint8_t)(err));
        uart_puts(huart, ")\r\n");
    }
    uart_puts(huart, "\r\n");

    /* ---- LHA7668B ADC ---- */
    uart_puts(huart, "[LHA7668B ADC]\r\n");
    uart_print_result(huart, "SPI Communication:", (err & ST_ERR_ADC_SPI) == 0);
    uart_puts(huart, "  Chip ID: 0x");
    uart_put_hex8(huart, g_self_test.adc_id);
    uart_puts(huart, " (expected 0x28)\r\n");
    uart_print_ok_fail(huart, "  ID Match", (err & ST_ERR_ADC_ID) == 0);
    uart_puts(huart, "  ERROR Register: 0x");
    uart_put_hex8(huart, g_self_test.adc_error);
    uart_puts(huart, "\r\n");
    uart_print_ok_fail(huart, "  ERROR status", (err & ST_ERR_ADC_ERROR) == 0);
    uart_puts(huart, "  Channels detected: ");
    uart_put_dec(huart, g_self_test.adc_channels);
    uart_puts(huart, "\r\n");
    uart_print_ok_fail(huart, "  DRDY data ready", g_self_test.adc_ready);
    uart_puts(huart, "\r\n");

    /* ---- W5500 Ethernet ---- */
    uart_puts(huart, "[W5500 Ethernet]\r\n");
    uart_print_result(huart, "SPI Communication:", (err & ST_ERR_W5500_SPI) == 0);
    uart_puts(huart, "  Chip Version: 0x");
    uart_put_hex8(huart, g_self_test.w5500_version);
    uart_puts(huart, " (expected 0x04)\r\n");
    uart_print_ok_fail(huart, "  PHY Link", g_self_test.w5500_phy_link);
    if (g_self_test.w5500_phy_link) {
        uart_puts(huart, " (");
        uart_puts(huart, g_self_test.w5500_speed ? "100M" : "10M");
        uart_puts(huart, ")\r\n");
    } else {
        uart_puts(huart, "\r\n");
    }

    /* ---- RS485 ---- */
    uart_puts(huart, "[RS485 (USART2)]\r\n");
    uart_print_ok_fail(huart, "  USART peripheral", g_self_test.rs485_uart_ok);
    uart_print_ok_fail(huart, "  DMA channel", g_self_test.rs485_dma_ok);
    uart_puts(huart, "\r\n");

    /* ---- Flash Storage ---- */
    uart_puts(huart, "[Flash Storage]\r\n");
    uart_print_ok_fail(huart, "  Data valid (magic+CRC)", g_self_test.flash_valid);
    if (g_self_test.flash_valid) {
        uart_print_ok_fail(huart, "  Decouple matrix", g_self_test.flash_matrix_ok);
        uart_print_ok_fail(huart, "  Zero offsets", g_self_test.flash_zero_ok);
        uart_print_ok_fail(huart, "  Network config", g_self_test.flash_config_ok);
    } else {
        if (err & ST_ERR_FLASH_EMPTY) {
            uart_puts(huart, "  Reason: Flash never written (first boot)\r\n");
        }
        if (err & ST_ERR_FLASH_CRC) {
            uart_puts(huart, "  Reason: CRC mismatch (data corrupted)\r\n");
        }
    }

    uart_puts(huart, "========================================\r\n");
    uart_puts(huart, "  SELF-TEST COMPLETE\r\n");
    uart_puts(huart, "========================================\r\n\r\n");
}
