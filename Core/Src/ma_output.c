#include "ma_output.h"
#include "sensor_types.h"
#include "rs485.h"
#include "at24c04.h"
#include "crc.h"
#include <string.h>

/* 1kHz 系统时基，定义在 main.c */
extern volatile uint32_t g_tick_ms;

/* ================================================================
 *  Internal state
 * ================================================================ */

/* 每通道配置。数组下标 0..5 对应「通道 1..6」，
 * 也就对应 Modbus 帧里的 数据1..数据6（寄存器 0x000A..0x000F）。 */
typedef struct {
    float zero[MA_CHANNEL_NUM];
    float extreme[MA_CHANNEL_NUM];
} ma_config_t;

static ma_config_t g_ma;

/* 上一次下发 Modbus 帧的时刻 */
static uint32_t g_ma_last_send_ms;

/* ---- 小端 float 存取（EEPROM 里与波特率保持同一字节序） ---- */

static void f32_to_le(uint8_t *dst, float f)
{
    uint32_t u;
    memcpy(&u, &f, 4);
    dst[0] = (uint8_t)(u);
    dst[1] = (uint8_t)(u >> 8);
    dst[2] = (uint8_t)(u >> 16);
    dst[3] = (uint8_t)(u >> 24);
}

static float le_to_f32(const uint8_t *src)
{
    uint32_t u = ((uint32_t)src[3] << 24)
               | ((uint32_t)src[2] << 16)
               | ((uint32_t)src[1] << 8)
               | ((uint32_t)src[0]);
    float f;
    memcpy(&f, &u, 4);
    return f;
}

/* ================================================================
 *  Defaults / validation
 * ================================================================ */

/* 配置值的合理上限。超出即视为 EEPROM 数据损坏（同时挡住 Inf/NaN）；
 * 物理量程远达不到 1e9，不会误伤正常配置。 */
#define MA_CFG_VALUE_LIMIT      1.0e9f

/** 套用出厂默认：零值全 0，极值 通道 1~3 = 0.3、通道 4~6 = 0.03 */
static void ma_output_apply_defaults(void)
{
    uint8_t i;

    for (i = 0; i < MA_CHANNEL_NUM; i++) {
        g_ma.zero[i]    = MA_DEFAULT_ZERO;
        g_ma.extreme[i] = (i < MA_DEFAULT_SPLIT_CH) ? MA_DEFAULT_EXTREME_A
                                                    : MA_DEFAULT_EXTREME_B;
    }
}

/** float 合法性检查：排除 NaN / Inf / 离谱大值 */
static int f32_is_valid(float v)
{
    if (v != v) return 0;                       /* NaN（含 Inf-Inf） */
    if (v >  MA_CFG_VALUE_LIMIT) return 0;      /* +Inf 或越界       */
    if (v < -MA_CFG_VALUE_LIMIT) return 0;      /* -Inf 或越界       */
    return 1;
}

/** 判断一段数据是否全为 0xFF（AT24C04 擦除态 = 空白） */
static int buf_is_blank(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++) {
        if (buf[i] != 0xFFU) return 0;
    }
    return 1;
}

/* ================================================================
 *  Configuration
 * ================================================================ */

void ma_output_init(void)
{
    uint8_t mode = MA_OUTPUT_MODE_FRAME;

    /* 先铺出厂默认值，再由 EEPROM 覆盖；
       EEPROM 空白（全 0xFF）或数据非法时默认值即最终配置。 */
    ma_output_apply_defaults();
    (void)ma_output_load();

    if (at24c04_read(MA_MODE_ADDR, &mode, 1) == HAL_OK) {
        g_sys.output_mode = (mode == MA_OUTPUT_MODE_MODBUS)
                          ? MA_OUTPUT_MODE_MODBUS
                          : MA_OUTPUT_MODE_FRAME;
    } else {
        g_sys.output_mode = MA_OUTPUT_MODE_FRAME;
    }

    g_ma_last_send_ms = g_tick_ms;
}

void ma_output_set_zero(uint8_t ch, float value)
{
    if (ch < 1U || ch > MA_CHANNEL_NUM) return;
    if (value != value) return;                 /* NaN 保护 */
    g_ma.zero[ch - 1U] = value;
}

void ma_output_set_extreme(uint8_t ch, float value)
{
    if (ch < 1U || ch > MA_CHANNEL_NUM) return;
    if (value != value) return;                 /* NaN 保护 */
    g_ma.extreme[ch - 1U] = value;
}

int ma_output_get_cfg(uint8_t ch, float *zero, float *extreme)
{
    if (ch < 1U || ch > MA_CHANNEL_NUM) return 0;

    if (zero)    *zero    = g_ma.zero[ch - 1U];
    if (extreme) *extreme = g_ma.extreme[ch - 1U];
    return 1;
}

/* ================================================================
 *  Value mapping
 * ================================================================ */

float ma_output_current_for(uint8_t ch, float n)
{
    float z, e, e2, hi, lo, span, r;

    if (ch < 1U || ch > MA_CHANNEL_NUM) return MA_CURRENT_ZERO;
    if (n != n) return MA_CURRENT_ZERO;               /* NaN → 12mA */

    z  = g_ma.zero[ch - 1U];
    e  = g_ma.extreme[ch - 1U];
    e2 = z - e;                                       /* 另一个极值 */

    hi = (e >= e2) ? e : e2;                          /* 12 → 20mA 一端 */
    lo = (e >= e2) ? e2 : e;                          /* 4  → 12mA 一端 */

    if (n >= z) {
        span = hi - z;
        if (!(span > 0.0f)) return MA_CURRENT_ZERO;   /* 未配置或退化 */
        r = (n - z) / span;
        if (r > 1.0f) r = 1.0f;                       /* 超上端点钳位 */
        return MA_CURRENT_ZERO + (MA_CURRENT_MAX - MA_CURRENT_ZERO) * r;
    }

    span = z - lo;
    if (!(span > 0.0f)) return MA_CURRENT_ZERO;
    r = (z - n) / span;
    if (r > 1.0f) r = 1.0f;                           /* 超下端点钳位 */
    return MA_CURRENT_ZERO - (MA_CURRENT_ZERO - MA_CURRENT_MIN) * r;
}

uint16_t ma_output_raw_for(uint8_t ch, float n)
{
    float    ma = ma_output_current_for(ch, n);
    uint32_t v;

    if (ma != ma)                   ma = MA_CURRENT_ZERO;
    if (ma < MA_CURRENT_MIN)        ma = MA_CURRENT_MIN;
    if (ma > MA_CURRENT_MAX)        ma = MA_CURRENT_MAX;

    /* 从站默认小数点 3 位：13mA → 13000 → 0x32C8 */
    v = (uint32_t)(ma * MA_RAW_SCALE + 0.5f);
    if (v > 0xFFFFU) v = 0xFFFFU;

    return (uint16_t)v;
}

/* ================================================================
 *  Modbus RTU frame
 * ================================================================ */

void ma_output_build_frame(uint8_t *frame, uint16_t *len)
{
    uint16_t crc;
    uint8_t  i;

    if (!frame) return;

    frame[0] = MA_MB_SLAVE_ADDR;
    frame[1] = MA_MB_FUNC_WRITE_MULTI;
    frame[2] = (uint8_t)(MA_MB_REG_START >> 8);
    frame[3] = (uint8_t)(MA_MB_REG_START & 0xFFU);
    frame[4] = (uint8_t)(MA_MB_REG_COUNT >> 8);
    frame[5] = (uint8_t)(MA_MB_REG_COUNT & 0xFFU);
    frame[6] = (uint8_t)(MA_MB_REG_COUNT * 2U);        /* 字节数 = 寄存器数×2 */

    for (i = 0; i < MA_CHANNEL_NUM; i++) {
        float    n = g_sensor.force[i];                /* 通道 i+1 的检测值 */
        uint16_t v = ma_output_raw_for((uint8_t)(i + 1U), n);

        frame[7U + i * 2U] = (uint8_t)(v >> 8);        /* 大端：高字节在前 */
        frame[8U + i * 2U] = (uint8_t)(v & 0xFFU);
    }

    crc = crc16_modbus(frame, 19U);                    /* CRC 低字节在前 */
    frame[19] = (uint8_t)(crc & 0xFFU);
    frame[20] = (uint8_t)(crc >> 8);

    if (len) *len = MA_MB_FRAME_LEN;
}

void ma_output_poll(void)
{
    uint8_t  frame[MA_MB_FRAME_LEN];
    uint16_t len = 0;

    if (g_sys.send_mode != 1U || g_sys.output_mode != MA_OUTPUT_MODE_MODBUS) {
        g_ma_last_send_ms = g_tick_ms;      /* 空闲时保持计时基准 */
        return;
    }

    if ((uint32_t)(g_tick_ms - g_ma_last_send_ms) < MA_SEND_INTERVAL_MS) return;
    g_ma_last_send_ms = g_tick_ms;

    ma_output_build_frame(frame, &len);
    rs485_send_plain(frame, len);
}

/* ================================================================
 *  Persistence
 * ================================================================ */

int ma_output_save(void)
{
    uint8_t buf[MA_CFG_BYTES];
    uint8_t magic = MA_CFG_VALID_MAGIC;
    uint8_t i;

    for (i = 0; i < MA_CHANNEL_NUM; i++) {
        f32_to_le(buf + i * 8U,      g_ma.zero[i]);
        f32_to_le(buf + i * 8U + 4U, g_ma.extreme[i]);
    }

    if (at24c04_write(MA_CFG_ADDR, buf, MA_CFG_BYTES) != HAL_OK) return 0;
    if (at24c04_write(MA_CFG_VALID_ADDR, &magic, 1) != HAL_OK) return 0;
    return 1;
}

int ma_output_load(void)
{
    uint8_t buf[MA_CFG_BYTES];
    uint8_t magic = 0;
    uint8_t i;

    if (at24c04_read(MA_CFG_ADDR, buf, MA_CFG_BYTES) != HAL_OK) return 0;

    /* 读到全 0xFF —— EEPROM 空白，从未配置过。
       返回 0，调用方给出的出厂默认值即为最终配置。 */
    if (buf_is_blank(buf, MA_CFG_BYTES)) return 0;

    /* 非空白，但有效标志不对 —— 残缺或外来数据，不采信 */
    if (at24c04_read(MA_CFG_VALID_ADDR, &magic, 1) != HAL_OK) return 0;
    if (magic != MA_CFG_VALID_MAGIC) return 0;

    /* 先整体校验再落盘，避免半截非法数据污染运行配置 */
    for (i = 0; i < MA_CHANNEL_NUM; i++) {
        if (!f32_is_valid(le_to_f32(buf + i * 8U)))      return 0;
        if (!f32_is_valid(le_to_f32(buf + i * 8U + 4U))) return 0;
    }

    for (i = 0; i < MA_CHANNEL_NUM; i++) {
        g_ma.zero[i]    = le_to_f32(buf + i * 8U);
        g_ma.extreme[i] = le_to_f32(buf + i * 8U + 4U);
    }
    return 1;
}

int ma_output_save_mode(uint8_t mode)
{
    uint8_t v = (mode == MA_OUTPUT_MODE_MODBUS) ? MA_OUTPUT_MODE_MODBUS
                                               : MA_OUTPUT_MODE_FRAME;
    return (at24c04_write(MA_MODE_ADDR, &v, 1) == HAL_OK) ? 1 : 0;
}
