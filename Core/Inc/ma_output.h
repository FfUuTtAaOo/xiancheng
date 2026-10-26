#ifndef __MA_OUTPUT_H__
#define __MA_OUTPUT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ================================================================
 *  4-20mA 输出映射  —— 把 6 路力值线性映射成 4-20mA，
 *  再以 Modbus RTU 0x10 指令下发给毫安输出模块。
 *
 *  映射规则：
 *    用户为每个通道设定「零值 Z」和「极值 E」，
 *    另一个极值 = Z - E，于是得到两个端点：
 *        上端点 hi = max(E, Z-E)   下端点 lo = min(E, Z-E)
 *    零值 Z 恒对应 12mA：
 *        N 在 [Z, hi]  -> 线性映射到 12..20mA
 *        N 在 [lo, Z]  -> 线性映射到 4..12mA
 *    超出端点则钳位在 4mA / 20mA。
 *
 *  例：Z=0, E=8  → 端点 {0, -8}
 *        N=1 → 12 + 8*(1/8) = 13mA → 寄存器值 13000 = 0x32C8
 *  例：Z=1, E=10 → 端点 {1, -9}
 *        N 在 1..10   → 12..20mA
 *        N 在 -9..1   → 4..12mA
 *
 *  Modbus 帧（21 字节，一帧同时下发 6 路）：
 *    01 10 00 0A 00 06 0C
 *       [ch1_hi ch1_lo] ... [ch6_hi ch6_lo]
 *       CRC_L CRC_H
 *  寄存器值 = mA × 1000（从站默认小数点 3 位，13mA → 13000 → 32 C8）
 * ================================================================ */

/** 通道数 —— 与 6 轴传感器一致 */
#define MA_CHANNEL_NUM          6U

/** g_sys.output_mode 取值 */
#define MA_OUTPUT_MODE_FRAME    0U   /* 原 AA 55 02 [6×float32] 0D 0A 数据流 */
#define MA_OUTPUT_MODE_MODBUS   1U   /* Modbus RTU 4-20mA 输出帧            */

/** Modbus RTU 帧参数 */
#define MA_MB_SLAVE_ADDR        0x01U      /* 毫安输出模块从站地址          */
#define MA_MB_FUNC_WRITE_MULTI  0x10U      /* 功能码：写多个保持寄存器      */
#define MA_MB_REG_START         0x000AU    /* 寄存器起始地址 = 10           */
#define MA_MB_REG_COUNT         6U         /* 寄存器个数 = 通道数           */
#define MA_MB_FRAME_LEN         21U        /* 7 头 + 12 数据 + 2 CRC        */

/** 4-20mA 电流参数 */
#define MA_CURRENT_MIN          4.0f
#define MA_CURRENT_ZERO         12.0f
#define MA_CURRENT_MAX          20.0f
#define MA_RAW_SCALE            1000.0f    /* mA → 寄存器原始值 的倍数      */

/* ---- 出厂默认配置（EEPROM 为空白全 0xFF 时启用）--------------
 * 零值统一为 0；极值前 3 通道 0.3N、后 3 通道 0.03N，
 * 即量程分别为 {0, -0.3} 与 {0, -0.03}。 */
#define MA_DEFAULT_ZERO         0.0f
#define MA_DEFAULT_EXTREME_A    0.3f       /* 通道 1~3 默认极值             */
#define MA_DEFAULT_EXTREME_B    0.03f      /* 通道 4~6 默认极值             */
#define MA_DEFAULT_SPLIT_CH     3U         /* 前 N 通道用 A，其余用 B       */

/** 下发周期（ms）—— 6 帧 21 字节在 115200 下约 11ms，20ms 留足帧间静默 */
#define MA_SEND_INTERVAL_MS     200U

/** EEPROM 存储布局（AT24C04，512 字节）
 *
 *  以下为工程内**实际占用**的 EEPROM 逻辑地址（含其它模块），
 *  新增数据前务必确认无重叠：
 *
 *    0x00 + i*0x18 (i=0..4)  矩阵第 1~5 行   各 24 字节
 *    0x78                    串口波特率       4 字节
 *    0x7C                    输出单位(kg/N)   1 字节
 *    0x80                    矩阵第 6 行      24 字节   <-- 注意不是空区！
 *    0x98                    零位标定 force_zero[6]  24 字节
 *    0xBC / 0xC0 / 0xC4      fw版本 / magic / CRC（预留，当前未落盘）
 *    0xC8                    4-20mA 配置      48 字节   <-- 本模块
 *    0xF8                    4-20mA 有效标志  1 字节   <-- 本模块
 *    0xF9                    输出模式         1 字节   <-- 本模块
 */
#define MA_CFG_ADDR             0xC8U      /* 48 字节：ch1..ch6 各 zero+extreme */
#define MA_CFG_BYTES            (MA_CHANNEL_NUM * 8U)
#define MA_CFG_VALID_ADDR       0xF8U      /* 1 字节有效标志                */
#define MA_CFG_VALID_MAGIC      0x5AU
#define MA_MODE_ADDR            0xF9U      /* 1 字节输出模式                */

/**
 * 初始化：从 EEPROM 恢复 6 路零值/极值与输出模式。
 *
 * EEPROM 中该配置区为空白（整片擦除态，全 0xFF）或数据非法时，
 * 自动套用出厂默认：零值全 0，极值 通道 1~3 = 0.3、通道 4~6 = 0.03。
 * 需在 at24c04 初始化之后调用。
 */
void ma_output_init(void);

/**
 * 设定某通道的零值（12mA 锚点）。
 * @param ch  通道号 1..6；越界直接忽略
 */
void ma_output_set_zero(uint8_t ch, float value);

/**
 * 设定某通道的极值（一个端点，另一端点为 零值-极值）。
 * @param ch  通道号 1..6；越界直接忽略
 */
void ma_output_set_extreme(uint8_t ch, float value);

/**
 * 读回某通道的配置。
 * @param ch       通道号 1..6
 * @param zero     输出：零值
 * @param extreme  输出：极值
 * @return 1 = 成功，0 = 通道号越界
 */
int ma_output_get_cfg(uint8_t ch, float *zero, float *extreme);

/**
 * 按当前配置把力值 n 换算成输出电流。
 * @param ch  通道号 1..6
 * @param n   检测值（单位与 g_sensor.force 一致）
 * @return 4.0 ~ 20.0 的电流值(mA)；配置退化或 n 为 NaN 时返回 12.0
 */
float ma_output_current_for(uint8_t ch, float n);

/**
 * 把力值 n 换算成 Modbus 寄存器原始值（mA × 1000）。
 * @return 4000 ~ 20000
 */
uint16_t ma_output_raw_for(uint8_t ch, float n);

/**
 * 用当前 6 路力值构造一帧 Modbus RTU 写多寄存器指令。
 * @param frame  输出缓冲区，至少 MA_MB_FRAME_LEN 字节
 * @param len    输出：实际帧长（21）
 */
void ma_output_build_frame(uint8_t *frame, uint16_t *len);

/**
 * 20ms 周期轮询：条件满足时下发一帧 Modbus 指令。
 * 由主循环无条件调用，内部自行判断 send_mode / output_mode。
 */
void ma_output_poll(void);

/** 把 6 路零值/极值写入 EEPROM。返回 1 成功、0 失败 */
int ma_output_save(void);

/**
 * 从 EEPROM 恢复 6 路零值/极值。
 * @return 1 = 读到有效配置；0 = 未配置（空白全 0xFF）、校验失败
 *             或数据非法 —— 此时调用方应保留默认值
 */
int ma_output_load(void);

/** 保存输出模式到 EEPROM。返回 1 成功、0 失败 */
int ma_output_save_mode(uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __MA_OUTPUT_H__ */
