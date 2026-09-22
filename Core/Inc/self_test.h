#ifndef __SELF_TEST_H__
#define __SELF_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f1xx_hal.h"   /* UART_HandleTypeDef */

/* ================================================================
 *  开机自检模块 (Boot Self-Test)
 *
 *  检查项目:
 *    1. LHA7668B ADC — SPI 通信、ID 寄存器、ERROR 寄存器
 *    2. W5500 以太网 — 版本寄存器、PHY 链路状态
 *    3. RS485 — UART/USART 初始化状态
 *    4. Flash 存储 — Magic/CRC 校验、矩阵/零点/配置数据摘要
 *
 *  自检结果可通过 Modbus Holding Register 读取，
 *  也可通过 UART 调试串口输出。
 * ================================================================ */

/* ---- 自检错误码 (bitmap, 0 = OK) ---- */
#define ST_ERR_NONE             0x00000000U

/* LHA7668B ADC 错误 (bit 0-7) */
#define ST_ERR_ADC_SPI          0x00000001U   /* SPI 通信失败             */
#define ST_ERR_ADC_ID           0x00000002U   /* ID 寄存器不匹配          */
#define ST_ERR_ADC_ERROR        0x00000004U   /* ERROR 寄存器有置位       */
#define ST_ERR_ADC_NODATA       0x00000008U   /* DRDY 无数据 (超时)       */
#define ST_ERR_ADC_RESERVED     0x000000F0U   /* reserved                 */

/* W5500 以太网错误 (bit 8-15) */
#define ST_ERR_W5500_SPI        0x00000100U   /* SPI 通信失败             */
#define ST_ERR_W5500_VERSION    0x00000200U   /* 版本号不匹配 (期望0x04)  */
#define ST_ERR_W5500_PHY        0x00000400U   /* PHY 链路未建立           */
#define ST_ERR_W5500_SOCKET     0x00000800U   /* Socket 初始化/监听失败   */
#define ST_ERR_W5500_RESERVED   0x0000F000U   /* reserved                 */

/* RS485 错误 (bit 16-19) */
#define ST_ERR_RS485_UART       0x00010000U   /* USART 外设状态异常       */
#define ST_ERR_RS485_DMA        0x00020000U   /* DMA 状态异常             */
#define ST_ERR_RS485_RESERVED   0x000C0000U   /* reserved                 */

/* Flash 存储错误 (bit 20-23) */
#define ST_ERR_FLASH_EMPTY      0x00100000U   /* Flash 未写入 (首次上电)  */
#define ST_ERR_FLASH_CRC        0x00200000U   /* Flash 数据 CRC 校验失败  */
#define ST_ERR_FLASH_RESERVED   0x00C00000U   /* reserved                 */

/* ---- 自检结果结构体 ---- */
typedef struct {
    uint32_t error_flags;       /* 错误位图 (0 = 全部通过)            */

    /* LHA7668B ADC 信息 */
    uint8_t  adc_id;            /* ID 寄存器值 (LHA7668B-8 = 0x28)   */
    uint8_t  adc_error;         /* ERROR 寄存器 bit[7:0] 低字节      */
    uint8_t  adc_channels;      /* 检测到的通道数                     */
    uint8_t  adc_ready;         /* 1 = DRDY 正常                     */

    /* W5500 信息 */
    uint8_t  w5500_version;     /* 版本寄存器值                       */
    uint8_t  w5500_phy_link;    /* 1 = PHY link up                   */
    uint8_t  w5500_sockets_ok;  /* 成功初始化的 Socket 数             */
    uint8_t  w5500_speed;       /* 0=10M, 1=100M (PHYCFGR bit2)     */

    /* RS485 信息 */
    uint8_t  rs485_uart_ok;     /* 1 = USART 外设正常                 */
    uint8_t  rs485_dma_ok;      /* 1 = DMA 正常                       */
    uint8_t  _pad[2];

    /* Flash 存储信息 */
    uint8_t  flash_valid;       /* 1 = Flash 数据有效 (magic+CRC)    */
    uint8_t  flash_matrix_ok;   /* 1 = 矩阵数据非全零/全FF           */
    uint8_t  flash_zero_ok;     /* 1 = 零点数据合理                   */
    uint8_t  flash_config_ok;   /* 1 = 配置数据合理 (IP 非 0.0.0.0) */
} self_test_t;

/* ---- 全局自检结果 (extern) ---- */
extern self_test_t g_self_test;

/**
 * 通过 UART 调试串口输出自检报告。
 * 调用 self_test_run_all() 后调用此函数打印结果。
 * @param huart  调试串口句柄 (通常为 huart2)
 */
void self_test_print_report(UART_HandleTypeDef *huart);

/* ---- 单项自检 (可由主循环调用以更新状态) ---- */

/** LHA7668B ADC 自检 */
uint32_t self_test_adc(void);

/** RS485 自检 */
uint32_t self_test_rs485(void);

/** Flash 存储自检 (不擦除，只读) */
uint32_t self_test_flash(void);

#ifdef __cplusplus
}
#endif

#endif /* __SELF_TEST_H__ */
