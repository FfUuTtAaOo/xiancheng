/******************************************************************************
 * 版权所有：苏州领慧立芯科技有限公司
 * Copyright (c) 2020-2025 Suzhou Legendsemi Technology Co., Ltd.
 ******************************************************************************
 * All rights reserved. Distributed under MIT license.
 * The file is encoded in UTF-8 without signature.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lha7668.h"
#include "spi.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_spi.h"

/* Constant ------------------------------------------------------------------*/
const uint8_t LHA7668_REG_SIZE[0x39] = {
    0x01, 0x02, 0x03, 0x03, 0x02, 0x01, 0x03, 0x03, 0x01, // 0x00 ~ 0x08
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,       // 0x09 ~ 0x18
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, // 0x19 ~ 0x20
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, // 0x21 ~ 0x28
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, // 0x29 ~ 0x30
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03  // 0x31 ~ 0x38
};

/* Applications --------------------------------------------------------------*/
/**
 * @brief  	应用层。分配LHA7668的Control IO输入管脚，建议CS必须，SYNC可选
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void LHL_LHA7668_Init(lhl_lha7668_ctx_t *ctx) {
    // 操作句柄的函数指针赋值
    ctx->read_write = LHA7668_Platform_ReadWrite;
    ctx->set = LHA7668_Platform_Set;
    ctx->reset = LHA7668_Platform_Reset;
    ctx->io_alloc = LHA7668_Platform_Init;
    ctx->delay_us = LHA7668_Platform_Delay_uS;

    // 设置默认IO端口为NULL，默认管脚为0
    ctx->io.cs.port = NULL;
    ctx->io.cs.pin = 0;
    ctx->io.sync.port = NULL;
    ctx->io.sync.pin = 0;

    // 调用初始化函数分配实际IO管脚
    ctx->io_alloc(ctx);
}

/**
 * @brief
 *应用层。对LHA7668-8/-4连续写入64个1，可以将器件的电路和串行接口复位。
 *			这将复位逻辑、数字滤波器和模拟调制器，从而所有片内寄存器复位为默认值。
 *			上电时会自动执行复位操作。复位需要90个MCLK周期的时间。
 *			状态寄存器中的 POR_FLAG
 *位在复位启动时置1，在复位完成时置0。 如果 SCLK
 *线路上的噪声导致串行接口变为异步状态，则需要执行复位以恢复同步功能。
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	Error_Status：	0->SUCCESS，其他->ERROR
 */
void LHL_LHA7668_Reset(lhl_lha7668_ctx_t *ctx) {
    // 发送至少8个字节的全1来复位芯片
    ctx->size = LHA7668_RESET_COUNT;

    for (uint8_t i = 0; i < ctx->size; i++) {
        ctx->tx.data[i] = LHA7668_BYTE_1;
    }

    __LHL_LHA7668_Read_Write_Raw(ctx);

    // 复位之后至少等待600uS，推荐1mS
    ctx->delay_us(1000);
}

/**
 * @brief  	应用层。读取芯片ID
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	uint8_t         lha7668芯片ID
 */
uint8_t LHL_LHA7668_Get_ID(lhl_lha7668_ctx_t *ctx) {
    // 调用读取函数，写入ID寄存器地址和宽度
    LHL_LHA7668_Read_Register(ctx, LHA7668_ID, LHA7668_ID_SIZE);

    return ctx->rx.data[1];
}

/**
 * @brief  	应用层。设置通道的正负输入端、对应配置和使能位
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void LHL_LHA7668_Set_Channel(lhl_lha7668_ctx_t *ctx, uint8_t channel) {
    ctx->CHANNEL.REG = 0;
    ctx->CHANNEL.REG |=
        ((uint32_t)ctx->CHANNEL.ENABLE << LHA7668_CHANNEL_ENABLE_POS) |
        ((uint32_t)ctx->CHANNEL.SETUP << LHA7668_CHANNEL_SETUP_POS) |
        ((uint32_t)ctx->CHANNEL.AINP << LHA7668_CHANNEL_AINP_POS) |
        ((uint32_t)ctx->CHANNEL.AINM << LHA7668_CHANNEL_AINM_POS);
    LHL_LHA7668_Write_Register(ctx, channel, ctx->CHANNEL.REG,
                               LHA7668_CHANNEL_SIZE);
}

/**
 * @brief  	应用层。设置指定通道的开路检测功能
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void LHL_LHA7668_Set_Burnout(lhl_lha7668_ctx_t *ctx,
                             lha7668_setup_number_t setup,
                             lha7668_burnout_t magnitude) {
    uint32_t adc_config;

    adc_config = LHL_LHA7668_Read_Register(ctx, (LHA7668_CONFIG_0 + setup),
                                           LHA7668_CONFIG_SIZE);
    adc_config &= ~LHA7668_CONFIG_BURNOUT_MSK; // BURNOUT 位掩码

    adc_config |= ((uint32_t)(magnitude & 0x03) << LHA7668_CONFIG_BURNOUT_POS);

    LHL_LHA7668_Write_Register(ctx, (LHA7668_CONFIG_0 + setup), adc_config,
                               LHA7668_CONFIG_SIZE);
}

/**
 * @brief  	应用层。设置配置的各项参数，包括Config和Filter寄存器
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void LHL_LHA7668_Set_Setup(lhl_lha7668_ctx_t *ctx,
                           lha7668_setup_number_t setup) {
    // 若REF_SEL选择为内部基准，则自动开启内部基准源，可在REFOUT测得2.5V
    if (ctx->SETUP.REF_SEL == LHA7668_REF_INTERNAL) {
        LHL_LHA7668_Set_Reference(ctx, LHA7668_ENABLE);
    }

    // 基准从REFOUT引脚外部提供，则自动关闭内部基准源
    if (ctx->SETUP.REF_SEL == LHA7668_REF_REFOUT_IN) {
        LHL_LHA7668_Set_Reference(ctx, LHA7668_DISABLE);
    }

    // 写入Config寄存器
    ctx->SETUP.REG[0] = 0;
    ctx->SETUP.REG[0] |=
        ((uint32_t)ctx->SETUP.BIPOLAR << LHA7668_CONFIG_BIPOLAR_POS) |
        ((uint32_t)ctx->SETUP.REF_BUFP << LHA7668_CONFIG_REF_BUFP_POS) |
        ((uint32_t)ctx->SETUP.REF_BUFM << LHA7668_CONFIG_REF_BUFM_POS) |
        ((uint32_t)ctx->SETUP.AIN_BUFP << LHA7668_CONFIG_AIN_BUFP_POS) |
        ((uint32_t)ctx->SETUP.AIN_BUFM << LHA7668_CONFIG_AIN_BUFM_POS) |
        ((uint32_t)(ctx->SETUP.REF_SEL & 0x3) << LHA7668_CONFIG_REF_SEL_POS) |
        ((uint32_t)ctx->SETUP.PGA << LHA7668_CONFIG_PGA_POS);
    LHL_LHA7668_Write_Register(ctx, (LHA7668_CONFIG_0 + setup),
                               ctx->SETUP.REG[0], LHA7668_CONFIG_SIZE);

    // 写入Filter寄存器
    ctx->SETUP.REG[1] = 0;
    ctx->SETUP.REG[1] |=
        ((uint32_t)ctx->SETUP.FILTER << LHA7668_FILTER_SINGLE_CYCLE_POS) |
        ((uint32_t)ctx->SETUP.FS << LHA7668_FILTER_FS_POS);
    LHL_LHA7668_Write_Register(ctx, (LHA7668_FILTER_0 + setup),
                               ctx->SETUP.REG[1], LHA7668_FILTER_SIZE);
}

/**
 * @brief  	应用层。设置ADC的通用参数，位于ADC_Control寄存器
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void LHL_LHA7668_Set_ADC(lhl_lha7668_ctx_t *ctx) {
    ctx->ADC_CTRL.REG =
        LHL_LHA7668_Read_Register(ctx, LHA7668_ADC_CTRL, LHA7668_ADC_CTRL_SIZE);
    // 注意REF_EN是和SETUP里面的REF_SEL同步配置的
    // 注意MODE是通过Start或者SetMode单独配置的
    ctx->ADC_CTRL.REG &= LHA7668_ADC_CTRL_REF_EN_MSK |
                         LHA7668_ADC_CTRL_MODE_MSK |
                         LHA7668_ADC_CTRL_CONT_READ_MSK;

    ctx->ADC_CTRL.REG |=
        ((uint32_t)ctx->ADC_CTRL.DOUT_RDY_DEL
         << LHA7668_ADC_CTRL_DOUT_RDY_DEL_POS) |
        ((uint32_t)ctx->ADC_CTRL.DATA_STATUS
         << LHA7668_ADC_CTRL_DATA_STATUS_POS) |
        ((uint32_t)ctx->ADC_CTRL.CS_EN << LHA7668_ADC_CTRL_CS_EN_POS) |
        ((uint32_t)ctx->ADC_CTRL.POWER_MODE
         << LHA7668_ADC_CTRL_POWER_MODE_POS) |
        ((uint32_t)ctx->ADC_CTRL.CLK_SEL << LHA7668_ADC_CTRL_CLK_SEL_POS);
    LHL_LHA7668_Write_Register(ctx, LHA7668_ADC_CTRL, ctx->ADC_CTRL.REG,
                               LHA7668_ADC_CTRL_SIZE);
}

/**
 * @brief  	应用层。设置ADC的诊断功能使能
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           error:          lhl_lha7668_error_t规定的可使能的诊断类型
 *           new_state:      使能或禁用
 * @retval 	NULL
 */
void LHL_LHA7668_Set_Diagnostics(lhl_lha7668_ctx_t *ctx, lha7668_error_t error,
                                 lha7668_functional_state new_state) {
    ctx->ERREN =
        LHL_LHA7668_Read_Register(ctx, LHA7668_ERREN, LHA7668_ERREN_SIZE);

    ctx->ERREN &= ~(1UL << error);

    ctx->ERREN |= (new_state << error);

    LHL_LHA7668_Write_Register(ctx, LHA7668_ERREN, ctx->ERREN,
                               LHA7668_ERREN_SIZE);
}

/**
 * @brief  	应用层。设置ADC为指定模式，并拉低CS（使能DRDY功能）
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           mode:asm        lha7668_mode_t定义的模式
 * @retval 	NULL
 */
void LHL_LHA7668_Start(lhl_lha7668_ctx_t *ctx, lha7668_mode_t mode) {
    //
    LHL_LHA7668_Set_Mode(ctx, mode);

    // 兼容CS_EN设计，确保CS是高->低电平
    ctx->set(ctx->io.cs.port, ctx->io.cs.pin);
    ctx->reset(ctx->io.cs.port, ctx->io.cs.pin);
}

/**
 * @brief  	应用层。拉低CS（使能DRDY功能）
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void LHL_LHA7668_Resume(lhl_lha7668_ctx_t *ctx) {
    // 兼容CS_EN设计，确保CS是高->低电平
    ctx->set(ctx->io.cs.port, ctx->io.cs.pin);
    ctx->reset(ctx->io.cs.port, ctx->io.cs.pin);
}

/**
 * @brief  	应用层。设置ADC为IDLE模式，并拉高CS
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void LHL_LHA7668_Stop(lhl_lha7668_ctx_t *ctx) {
    // 将LHA7668置于IDLE模式
    LHL_LHA7668_Set_Mode(ctx, LHA7668_MODE_IDLE);

    // 拉高CS
    ctx->set(ctx->io.cs.port, ctx->io.cs.pin);
}

/**
 * @brief
 * 应用层。读取ADC数据，并将结果存入data[4]，前3字节是数据，第4字节是状态位（若有）
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void LHL_LHA7668_Get_Data(lhl_lha7668_ctx_t *ctx) {
    switch (ctx->ADC_CTRL.MODE) {
        case LHA7668_MODE_SINGLE_SHOT:
        case LHA7668_MODE_CONT_CONV:
            // 数据命令读模式
            ctx->tx.parts.address = LHA7668_DATA;
            ctx->tx.parts.address &= ~LHA7668_COMM_WEN_MSK;
            ctx->tx.parts.address |= LHA7668_COMM_R_W_MSK;
            ctx->size = LHA7668_DATA_SIZE + LHA7668_COMM_SIZE;
            // 判断STATUS是否使能
            if (ctx->ADC_CTRL.DATA_STATUS == LHA7668_ENABLE) {
                ctx->size++;
            }
            // 判断CRC是否使能

            // 命令读数据时，保持DIN为1
            for (uint8_t i = 0; i < ctx->size; i++) {
                ctx->tx.parts.content[i] = LHA7668_BYTE_1;
            }
            __LHL_LHA7668_Read_Data(ctx);
            ctx->data = (uint32_t)0 | ((uint32_t)ctx->rx.data[1] << 16) |
                        ((uint32_t)ctx->rx.data[2] << 8) |
                        ((uint32_t)ctx->rx.data[3]);
            ctx->status = ctx->rx.data[4];
            break;
        case LHA7668_MODE_CONT_READ:
            // 数据连续读模式
            ctx->tx.parts.address = LHA7668_COMM;
            ctx->size = LHA7668_DATA_SIZE;
            // 判断STATUS是否使能
            if (ctx->ADC_CTRL.DATA_STATUS == LHA7668_ENABLE) {
                ctx->size++;
            }
            // 判断STATUS是否使能
            if (ctx->ERREN | LHA7668_ERREN_SPI_CRC_ERR_EN_MSK) {
                ctx->size++;
            }
            // 连续读数据时，保持DIN为0
            for (uint8_t i = 0; i < ctx->size; i++) {
                ctx->tx.parts.content[i] = LHA7668_BYTE_0;
            }
            __LHL_LHA7668_Read_Data(ctx);
            ctx->data = (uint32_t)0 | ((uint32_t)ctx->rx.data[0] << 16) |
                        ((uint32_t)ctx->rx.data[1] << 8) |
                        ((uint32_t)ctx->rx.data[2]);
            ctx->status = ctx->rx.data[3];
            ctx->crc = ctx->rx.data[4];
            break;
        case LHA7668_MODE_INTL_OFFSET_CAL:
        case LHA7668_MODE_INTL_GAIN_CAL:
        case LHA7668_MODE_SYS_OFFSET_CAL:
        case LHA7668_MODE_SYS_GAIN_CAL:
        case LHA7668_MODE_IDLE:
        case LHA7668_MODE_STANDY:
        case LHA7668_MODE_SHUT_DOWN:
            break;
    }
    //    return 0;   // 0: Success; -1: Error;
}

/**
 * @brief  	应用层。将原始ADC Raw
 * Data根据BIPOLAR和GAIN和REF基准源转换为电压（mV）
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           raw:            原始ADC Raw Data
 *           bipolar:        单极性或双极性数据
 *           gain:           通道增益
 *           ref_mV:         基准源电压（单位：毫伏）
 * @retval 	float:          返回电压值（单位：毫伏）
 */
#define PGA_GAIN_CALC(s) (((s) == 0) ? 1 : (1U << ((s) + 1)) >> 1)
#define BIPOLAR_COEFFICIENT (1.0f / 8388608.0f)   // 1/(2^23)
#define UNIPOLAR_COEFFICIENT (1.0f / 16777216.0f) // 1/(2^24)
float LHL_LHA7668_Get_mVoltage(const uint32_t raw,
                               const lha7668_bipolar_t bipolar,
                               const lha7668_pga_t gain, const float ref_mV) {
    // Pre-calculate common factor to avoid repeated division
    // const float scale = ref_mV / (float)PGA_GAIN_CALC(gain);
const float scale = ref_mV;
    // Use bit shift instead of power-of-two division and avoid branching where
    // possible
    return bipolar
               ? (float)((int32_t)raw - 0x800000) * BIPOLAR_COEFFICIENT * scale
               : (float)raw * UNIPOLAR_COEFFICIENT * scale;
}

/**
 * @brief  	应用层。将原始ADC Raw
 * Data根据BIPOLAR和GAIN和参考电阻转换为电阻（Ω）
 */
float LHL_LHA7668_Get_Ohm(const uint32_t raw, const lha7668_bipolar_t bipolar,
                          const lha7668_pga_t gain, const float ref_Ohm) {
    const float scale = ref_Ohm / (float)PGA_GAIN_CALC(gain);

    return bipolar
               ? (float)((int32_t)raw - 0x800000) * BIPOLAR_COEFFICIENT * scale
               : (float)raw * UNIPOLAR_COEFFICIENT * scale;
}
/**
 * @brief  	应用层。将原始ADC Raw Data换算为片内温度值（℃）
 */
#define TEMP_COEFFICIENT (1.0f / 13584.0f) // 温度预计算转换系数（1/13584）
#define TEMP_OFFSET 272.5f
float LHL_LHA7668_Get_Temperature(const uint32_t raw) {
    // 使用带符号运算处理原始数据偏移
    return ((float)((int32_t)raw - 0x800000) * TEMP_COEFFICIENT) - TEMP_OFFSET;
}

/**
 * @brief  	应用层。设置ADC Mode
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           mode:           lha7668_mode_t定义的操作模式
 * @retval 	NULL
 */
void LHL_LHA7668_Set_Mode(lhl_lha7668_ctx_t *ctx, lha7668_mode_t mode) {
    lha7668_adc_ctrl_t adc_ctrl;

    adc_ctrl.REG =
        LHL_LHA7668_Read_Register(ctx, LHA7668_ADC_CTRL, LHA7668_ADC_CTRL_SIZE);
    adc_ctrl.REG &=
        ~(LHA7668_ADC_CTRL_MODE_MSK |
          LHA7668_ADC_CTRL_CONT_READ_MSK); // MODE位和CONT_READ位掩码

    adc_ctrl.REG |=
        ((uint32_t)(mode & 0x0F) << 2) | ((uint32_t)(mode & 0x10) << 7);

    ctx->ADC_CTRL.MODE = mode;

    LHL_LHA7668_Write_Register(ctx, LHA7668_ADC_CTRL, adc_ctrl.REG,
                               LHA7668_ADC_CTRL_SIZE);
}

/**
 * @brief  	应用层。设置ADC Power Mode
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           mode:           lha7668_mode_t定义的操作模式
 * @retval 	NULL
 */
void LHL_LHA7668_Set_Power(lhl_lha7668_ctx_t *ctx, lha7668_power_mode_t power) {
    lha7668_adc_ctrl_t adc_ctrl;

    adc_ctrl.REG =
        LHL_LHA7668_Read_Register(ctx, LHA7668_ADC_CTRL, LHA7668_ADC_CTRL_SIZE);
    adc_ctrl.REG &= ~LHA7668_ADC_CTRL_POWER_MODE_MSK; // POWER MODE位掩码

    adc_ctrl.REG |=
        ((uint32_t)(power & 0x03) << LHA7668_ADC_CTRL_POWER_MODE_POS);

    LHL_LHA7668_Write_Register(ctx, LHA7668_ADC_CTRL, adc_ctrl.REG,
                               LHA7668_ADC_CTRL_SIZE);
}

/**
 * @brief  	应用层。设置VBias输出
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           vbias:          0~15，对应偏置电压输出通道(AIN0~AIN15)
 *           new_state:      ENABLE->输出；DISABLE->关闭
 * @retval 	NULL
 */
void LHL_LHA7668_Set_VBias(lhl_lha7668_ctx_t *ctx, lha7668_vbias_t vbias,
                           lha7668_functional_state new_state) {
    uint32_t io_control_2;
    io_control_2 =
        LHL_LHA7668_Read_Register(ctx, LHA7668_IO_CTRL2, LHA7668_IO_CTRL2_SIZE);
    if (new_state == LHA7668_ENABLE) {
        io_control_2 |= (1UL << vbias);
    } else {
        io_control_2 &= ~(1UL << vbias);
    }
    LHL_LHA7668_Write_Register(ctx, LHA7668_IO_CTRL2, io_control_2,
                               LHA7668_IO_CTRL2_SIZE);
}

/**
 * @brief  	应用层。同时设置IOut0和IOout1输出，双激励电流大小一致。
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           iout0_channel:  0~15，对应激励电流0输出通道(AIN0~AIN15)
 *           iout1_channel:  0~15，对应激励电流1输出通道(AIN0~AIN15)
 *           current:        激励电流大小
 * @retval 	NULL
 */
void LHL_LHA7668_Set_IOut(lhl_lha7668_ctx_t *ctx, lha7668_iout_t iout0_channel,
                          lha7668_iout_t iout1_channel,
                          lha7668_current_t current) {
    uint32_t io_control_1;
    io_control_1 =
        LHL_LHA7668_Read_Register(ctx, LHA7668_IO_CTRL1, LHA7668_IO_CTRL1_SIZE);
    io_control_1 &=
        ~(LHA7668_IO_CTRL1_IOUT0_CH_MSK | LHA7668_IO_CTRL1_IOUT0_MSK |
          LHA7668_IO_CTRL1_IOUT1_CH_MSK | LHA7668_IO_CTRL1_IOUT1_MSK);
    io_control_1 |= (iout0_channel << LHA7668_IO_CTRL1_IOUT0_CH_POS) |
                    (iout1_channel << LHA7668_IO_CTRL1_IOUT1_CH_POS) |
                    (current << LHA7668_IO_CTRL1_IOUT0_POS) |
                    (current << LHA7668_IO_CTRL1_IOUT1_POS);
    LHL_LHA7668_Write_Register(ctx, LHA7668_IO_CTRL1, io_control_1,
                               LHA7668_IO_CTRL1_SIZE);
}

/**
 * @brief  	应用层。单独设置IOut0输出
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           iout:           0~15，对应激励电流输出通道(AIN0~AIN15)
 *           current:        激励电流大小
 * @retval 	NULL
 */
void LHL_LHA7668_Set_IOut0(lhl_lha7668_ctx_t *ctx, lha7668_iout_t iout0_channel,
                           lha7668_current_t current) {
    uint32_t io_control_1;
    io_control_1 =
        LHL_LHA7668_Read_Register(ctx, LHA7668_IO_CTRL1, LHA7668_IO_CTRL1_SIZE);
    io_control_1 &=
        ~(LHA7668_IO_CTRL1_IOUT0_CH_MSK | LHA7668_IO_CTRL1_IOUT0_MSK);
    io_control_1 |= (iout0_channel << LHA7668_IO_CTRL1_IOUT0_CH_POS) |
                    (current << LHA7668_IO_CTRL1_IOUT0_POS);
    LHL_LHA7668_Write_Register(ctx, LHA7668_IO_CTRL1, io_control_1,
                               LHA7668_IO_CTRL1_SIZE);
}

/**
 * @brief  	应用层。单独设置IOut1输出
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           iout:           0~15，对应激励电流输出通道(AIN0~AIN15)
 *           current:        激励电流大小
 * @retval 	NULL
 */
void LHL_LHA7668_Set_IOut1(lhl_lha7668_ctx_t *ctx, lha7668_iout_t iout1_channel,
                           lha7668_current_t current) {
    uint32_t io_control_1;
    io_control_1 =
        LHL_LHA7668_Read_Register(ctx, LHA7668_IO_CTRL1, LHA7668_IO_CTRL1_SIZE);
    io_control_1 &=
        ~(LHA7668_IO_CTRL1_IOUT1_CH_MSK | LHA7668_IO_CTRL1_IOUT1_MSK);
    io_control_1 |= (iout1_channel << LHA7668_IO_CTRL1_IOUT1_CH_POS) |
                    (current << LHA7668_IO_CTRL1_IOUT1_POS);
    LHL_LHA7668_Write_Register(ctx, LHA7668_IO_CTRL1, io_control_1,
                               LHA7668_IO_CTRL1_SIZE);
}

void LHL_LHA7668_Set_GPIO(lhl_lha7668_ctx_t *ctx) {
}

void LHL_LHA7668_Get_GPIO(lhl_lha7668_ctx_t *ctx) {
}

/**
 * @brief  	应用层。返回指定标志位的当前状态。
 * @param  	*ctx：                   lhl_lha7668_ctx_t操作句柄
 *           lha7668_status_flag_t：  需要读取的标志位
 * @retval 	lha7668_flag_status：    LHA7668_SET = 1; LHA7668_RESET = 0
 */
lha7668_flag_status LHL_LHA7668_Get_Flag(lhl_lha7668_ctx_t *ctx,
                                         const lha7668_status_flag_t flag) {
    lha7668_status_t adc_status;
    adc_status.REG = (uint8_t)LHL_LHA7668_Read_Register(ctx, LHA7668_STATUS,
                                                        LHA7668_STATUS_SIZE);
    if (adc_status.REG & flag) {
        return LHA7668_SET;
    } else {
        return LHA7668_RESET;
    }
}

/**
 * @brief  	应用层。使能或禁用内部基准电压。
 *           此位置1时，内部基准电压源使能，并通过REFOUT引脚输出。
 *           此位清0时，内部基准电压源禁用。
 * @param  	*ctx：           lhl_lha7668_ctx_t操作句柄
 *           new_state：      ENABLE->使能，DISABLE->禁用
 * @retval 	NULL
 */
void LHL_LHA7668_Set_Reference(lhl_lha7668_ctx_t *ctx,
                               lha7668_functional_state new_state) {
    lha7668_adc_ctrl_t adc_ctrl;
    adc_ctrl.REG =
        LHL_LHA7668_Read_Register(ctx, LHA7668_ADC_CTRL, LHA7668_ADC_CTRL_SIZE);
    adc_ctrl.REG &= ~LHA7668_ADC_CTRL_REF_EN_MSK;
    adc_ctrl.REG |= ((uint32_t)new_state << LHA7668_ADC_CTRL_REF_EN_POS);
    LHL_LHA7668_Write_Register(ctx, LHA7668_ADC_CTRL, adc_ctrl.REG,
                               LHA7668_ADC_CTRL_SIZE);
}

/**
 * @brief  	应用层。读取LHA7668寄存器内容，返回值存放在buf指向的数组
 * @param  	*ctx：           lhl_lha7668_ctx_t操作句柄
 *           start_address:  读取的寄存器 - 起始地址
 *           edn_address:    读取的寄存器 - 结束地址
 *           *buf:           存放读取到的寄存器数据。lha7668全部数组长度0x39
 * @retval 	NULL
 */
void LHL_LHA7668_Dump_Register(lhl_lha7668_ctx_t *ctx,
                               const uint8_t start_address,
                               const uint8_t end_address, uint32_t *buf) {
    for (uint8_t i = start_address; i < (end_address + 1);
         i++) // 循环读取所有地址
    {
        LHL_LHA7668_Read_Register(ctx, i, 3); // LHA7668寄存器读取宽度24b
        // 取rx原始数据，首字节是无效数据，并需要根据小端序转为INT 32b长数据
        *buf++ = __LHL_Array_to_Int(&ctx->rx.data[1]);
    }
}

void LHL_LHA7668_Dump_Register_CRC(lhl_lha7668_ctx_t *ctx,
                                   const uint8_t start_address,
                                   const uint8_t end_address, uint32_t *buf,
                                   uint8_t *crc) {
    for (uint8_t i = start_address; i < (end_address + 1);
         i++) // 循环读取所有地址
    {
        LHL_LHA7668_Read_Register(ctx, i, 3); // LHA7668寄存器读取宽度24b
        // 取rx原始数据，首字节是无效数据，并需要根据小端序转为INT 32b长数据
        *buf++ = __LHL_Array_to_Int(&ctx->rx.data[1]);
        //*crc++ = ctx->crc; //读取的CRC保存至ctx->crc内
        *crc++ = calculate_crc8(&ctx->rx.data[0],
                                LHA7668_REG_SIZE[i] + 1); // 计算CRC值
    }
}

/**
 * @brief  	应用层。根据规格书通信协议，读取指定的单个寄存器内容。
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           address:        要读取的寄存器名称（地址）
 *           size:           寄存器实际宽度
 * @retval 	uint32_t: 返回寄存器内容，长度是32b，有效数据与寄存器宽度一致
 */
uint32_t LHL_LHA7668_Read_Register(lhl_lha7668_ctx_t *ctx,
                                   const uint8_t address, const uint8_t size) {
    ctx->tx.parts.address = address;
    ctx->tx.parts.address &=
        ~LHA7668_COMM_WEN_MSK; // 读取需要修改通信寄存器的WEN和R/W的MASK位
    ctx->tx.parts.address |= LHA7668_COMM_R_W_MSK;

    ctx->size =
        size + LHA7668_COMM_SIZE; // 发送SLCK时钟等于寄存器宽度 + 通信寄存器

    // 需要根据CRC状态修正SCLK
    if (ctx->ERREN | LHA7668_ERREN_SPI_CRC_ERR_EN_MSK) {
        ctx->size++;
    }

    for (uint8_t i = 0; i < size; i++) {
        ctx->tx.parts.content[i] = LHA7668_BYTE_1; // 定义Content，读取时发送全1
    }

    __LHL_LHA7668_Read_Write_Raw(ctx);

    // 需要根据CRC状态返回CRC值
    if (ctx->ERREN & LHA7668_ERREN_SPI_CRC_ERR_EN_MSK) {
        ctx->crc = ctx->rx.data[LHA7668_REG_SIZE[address] + 1];
    }

    // 将接收数据的首个DUMMY DATA修改为发送命令以方便CRC计算
    ctx->rx.data[0] = ctx->tx.parts.address;

    // 因lha7668寄存器宽度不定，需要调整后返回读取值
    uint32_t reg_r = 0;
    for (uint8_t j = 0; j < size; j++) {
        reg_r = reg_r << 8;
        reg_r |= ctx->rx.data[j + 1];
    }
    return reg_r;
}

/* Middleware ----------------------------------------------------------------*/
/**
 * @brief  	协议层。根据规格书通信协议，写入指定的单个寄存器内容。
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 *           address:        要写入的寄存器名称（地址）
 *           content: 要写入的寄存器内容，注意有效数据不可超过寄存器实际宽度
 *           size:           寄存器实际宽度
 * @retval 	NULL
 */
void LHL_LHA7668_Write_Register(lhl_lha7668_ctx_t *ctx, const uint8_t address,
                                const uint32_t content, const uint8_t size) {
    ctx->tx.parts.address = address;
    ctx->tx.parts.address &=
        ~LHA7668_COMM_WEN_MSK; // 写入操作需要修改通信寄存器的WEN和R/W的MASK位
    ctx->tx.parts.address &= ~LHA7668_COMM_R_W_MSK;

    ctx->size =
        size + LHA7668_COMM_SIZE; // 发送SLCK时钟等于寄存器宽度 + 通信寄存器

    uint32_t reg_w = content;
    for (uint8_t i = 0; i < size; i++) {
        ctx->tx.parts.content[size - (i + 1)] =
            (reg_w & 0xFF); // 因lha7668寄存器宽度不定，需要调整写入值
        reg_w = reg_w >> 8;
    }

    // 需要根据CRC状态修正SCLK
    if (ctx->ERREN | LHA7668_ERREN_SPI_CRC_ERR_EN_MSK) {
        ctx->tx.parts.content[size] =
            calculate_crc8(&ctx->tx.data[0], ctx->size);
        ctx->size++;
    }

    __LHL_LHA7668_Read_Write_Raw(ctx);
}

/* Middleware ----------------------------------------------------------------*/
/**
 * @brief  	时序层。控制cs管脚，按size长度，传输tx数据，接收rx数据
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void __LHL_LHA7668_Read_Write_Raw(lhl_lha7668_ctx_t *ctx) {
    // 传输指定长度的字节
    ctx->reset(ctx->io.cs.port, ctx->io.cs.pin);

    ctx->read_write(ctx->tx.data, ctx->rx.data, ctx->size);

    ctx->set(ctx->io.cs.port, ctx->io.cs.pin);
}

/**
 * @brief  	时序层，保证cs管脚为低，按size长度，写入并读取数据
 * @param  	*ctx：			lhl_lha7668_ctx_t操作句柄
 * @retval 	NULL
 */
void __LHL_LHA7668_Read_Data(lhl_lha7668_ctx_t *ctx) {
    // 读数据时，无CS控制，且需保证CS为低电平
    ctx->reset(ctx->io.cs.port, ctx->io.cs.pin);
    ctx->read_write(ctx->tx.data, ctx->rx.data, ctx->size);
}

/**
 * @brief  	ARM体系默认小端序，将MSB BYTE ARRAY转换为INT
 */
uint32_t __LHL_Array_to_Int(const uint8_t *data) {
    if (LHA7668_Little_Endian) {
        return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
               ((uint32_t)data[2] << 8) | (uint32_t)data[3];
    } else {
        return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
               ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
    }
}

/* Low Level Drivers ---------------------------------------------------------*/
/**
 * @brief  	底层。驱动函数原型区，由用户实现
 */
__LHA7668_WEAK void LHA7668_Platform_Init(void *handle) {
    // 如果不重写此函数，则不会控制CS和SYNC管脚
}

__LHA7668_WEAK void LHA7668_Platform_Set(const void *port, const uint32_t pin) {
    // 如果不需要驱动库控制CS，则不要重写此函数
    HAL_GPIO_WritePin((GPIO_TypeDef *)port, pin, GPIO_PIN_SET);
}

__LHA7668_WEAK void LHA7668_Platform_Reset(const void *port,
                                           const uint32_t pin) {
    HAL_GPIO_WritePin((GPIO_TypeDef *)port, pin, GPIO_PIN_RESET);

    // 如果不需要驱动库控制CS，则不要重写此函数
}

__LHA7668_WEAK int32_t LHA7668_Platform_ReadWrite(uint8_t *txdata,
                                                  uint8_t *rxdata,
                                                  const uint16_t size) {
    // SPI收发函数，必须由用户实现
    HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, size, 1000);
    return 0;
}

__LHA7668_WEAK void LHA7668_Platform_Delay_uS(volatile uint32_t cycles) {
    // 建议用户重写此函数，例如使用Timer，实现uS级别精确定时
    cycles = cycles * LHA7668_System_Core_Clock / 10;
    while (cycles--) {
        // 空循环体，内存屏障确保不被优化
        __LHA7668_NOTHING;
    }
}

/* Extra Function Drivers
 * -------------------------------------------------------*/

// PT100电阻-温度分界点（0℃~300℃范围示例）
typedef struct {
    float resistance;
    float temperature;
} __LHL_RTD_Segment;

static const __LHL_RTD_Segment __lhl_rtd_table[] = {
    { 80.31f, -50.0f}, // -50℃
    { 84.27f, -40.0f}, // -40℃
    { 88.22f, -30.0f}, // -30℃
    { 92.16f, -20.0f}, // -20℃
    { 96.09f, -10.0f}, // -10℃
    {100.00f,   0.0f}, // 0℃
    {103.90f,  10.0f}, // 10℃
    {107.79f,  20.0f}, // 20℃
    {111.67f,  30.0f}, // 30℃
    {115.54f,  40.0f}, // 40℃
    {119.40f,  50.0f}, // 50℃
    {138.51f, 100.0f}, // 100℃
    {157.33f, 150.0f}, // 150℃
    {175.86f, 200.0f}, // 200℃
    {194.10f, 250.0f}, // 250℃
    {212.05f, 300.0f}  // 300℃
};

#define RTD_TABLE_SIZE (sizeof(__lhl_rtd_table) / sizeof(__LHL_RTD_Segment))

float rtd_resistance_to_temperature(float resistance) {
    // 边界检查
    if (resistance < __lhl_rtd_table[0].resistance)
        return -275.15f; // 无效值
    if (resistance >= __lhl_rtd_table[RTD_TABLE_SIZE - 1].resistance)
        return __lhl_rtd_table[RTD_TABLE_SIZE - 1].temperature;

    // 查找所在区间
    for (uint8_t i = 1; i < RTD_TABLE_SIZE; i++) {
        if (resistance < __lhl_rtd_table[i].resistance) {
            const __LHL_RTD_Segment *lower = &__lhl_rtd_table[i - 1];
            const __LHL_RTD_Segment *upper = &__lhl_rtd_table[i];

            // 线性插值计算
            float delta_r = upper->resistance - lower->resistance;
            float delta_t = upper->temperature - lower->temperature;
            return lower->temperature +
                   (resistance - lower->resistance) * (delta_t / delta_r);
        }
    }

    return __lhl_rtd_table[RTD_TABLE_SIZE - 1].temperature; // 返回最大值
}

/******************************************************************************
 *函数描述:  K型 电压转温度(冷端温度为0)毫伏范围：-5.891 ~ +54.819
 *温度范围：  -199.933C° ~ +1370.057C°
 *输入变量:  mV 毫伏
 *返回变量:  temperature 温度
 ******************************************************************************/
// K型电压转温度系数
float Var_VtoT_K[3][10] = {
    {0, 2.5173462e1, -1.1662878, -1.0833638, -8.9773540 / 1e1, -3.7342377 / 1e1,
     -8.6632643 / 1e2, -1.0450598 / 1e2, -5.1920577 / 1e4},
    {0, 2.508355e1, 7.860106 / 1e2, -2.503131 / 1e1, 8.315270 / 1e2,
     -1.228034 / 1e2, 9.804036 / 1e4, -4.413030 / 1e5, 1.057734 / 1e6,
     -1.052755 / 1e8},
    {-1.318058e2, 4.830222e1, -1.646031, 5.464731 / 1e2, -9.650715 / 1e4,
     8.802193 / 1e6, -3.110810 / 1e8}
};
float K_VtoT(float mV) {
    unsigned char i = 0;
    float value = 0;
    if (mV >= -6.478 && mV < 0) {
        value = Var_VtoT_K[0][8];
        for (i = 8; i > 0; i--)
            value = mV * value + Var_VtoT_K[0][i - 1];
    } else if (mV >= 0 && mV < 20.644) {
        value = Var_VtoT_K[1][9];
        for (i = 9; i > 0; i--)
            value = mV * value + Var_VtoT_K[1][i - 1];
    } else if (mV >= 20.644 && mV <= 54.900) {
        value = Var_VtoT_K[2][6];
        for (i = 6; i > 0; i--)
            value = mV * value + Var_VtoT_K[2][i - 1];
    }
    return value;
}

/******************************************************************************
 *函数描述:  K型 温度转电压(冷端温度为0)
 *温度范围：  -270C° ~ +1370C°
 *毫伏范围：  -6.4577 ~ +54.8185
 *输入变量:  Temperature 温度
 *返回变量:  microVoltage 毫伏
 ******************************************************************************/
// K型温度转电压系数
float Var_TtoV_K[3][11] = {
    {0, 3.9450128025 / 1e2, 2.3622373598 / 1e5, -3.2858906784 / 1e7,
     -4.9904828777 / 1e9, -6.7509059173 / 1e11, -5.7410327428 / 1e13,
     -3.1088872894 / 1e15, -1.0451609365 / 1e17, -1.9889266878 / 1e20,
     -1.6322697486 / 1e23},
    {-1.7600413686 / 1e2, 3.8921204975 / 1e2, 1.8558770032 / 1e5,
     -9.9457592874 / 1e8, 3.1840945719 / 1e10, -5.6072844889 / 1e13,
     5.6075059059 / 1e16, -3.2020720003 / 1e19, 9.7151147152 / 1e23,
     -1.2104721275 / 1e26},
    {1.185976 / 1e1, -1.183432 / 1e4}
};
float K_TtoV(float T) {
    unsigned char i = 0;
    float value = 0;
    if ((T < -270) || (T > 1370.0)) {
        return 0;
    } else if (T < 0) {
        value = Var_TtoV_K[0][10];
        for (i = 10; i > 0; i--)
            value = T * value + Var_TtoV_K[0][i - 1];
        value += Var_TtoV_K[2][0] *
                 exp(Var_TtoV_K[2][1] * (T - 126.9686) * (T - 126.9686));
    } else // if(T >= 0 && T <= 1372)
    {
        value = Var_TtoV_K[1][9];
        for (i = 9; i > 0; i--)
            value = T * value + Var_TtoV_K[1][i - 1];
        value += Var_TtoV_K[2][0] *
                 exp(Var_TtoV_K[2][1] * (T - 126.9686) * (T - 126.9686));
    }
    return value;
}

// K型热电偶带冷端补偿温度计算函数
float calculate_k_type_temperature(float emf_mv, float cold_junction_temp) {
    // 冷端温度转电动势
    float cold_emf = K_TtoV(cold_junction_temp);

    // 总电动势 = 测量电动势 + 冷端补偿电动势
    float total_emf = emf_mv + cold_emf;

    return K_VtoT(total_emf);
}

/**
 * 计算CRC-8校验和 (多项式: x^8 + x^2 + x + 1)
 * @param data 数据指针
 * @param length 数据长度(字节数)
 * @return 计算出的CRC值
 */
uint8_t calculate_crc8(const uint8_t *data, uint8_t length) {
    uint8_t crc = 0x00;

    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ LHA7668_CRC8_POLY;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

int verify_crc8(const uint8_t *data, uint8_t length) {
    if (length == 0)
        return -1;

    uint8_t crc = calculate_crc8(data, length - 1);
    return (crc == data[length - 1]) ? 0 : -1;
}
