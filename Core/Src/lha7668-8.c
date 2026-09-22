#include "lha7668-8.h"

uint8_t adc_init_count = 0;

/* RDY 轮询超时上限：防止 ADC 未就绪 / SPI 异常时 while 死循环卡死
   每次轮询约 30us（4 字节 SPI @3MHz），500 次 ≈ 15ms */
#define LHA7668_RDY_WAIT_MAX   500

lhl_lha7668_ctx_t lha7668_ctx = { 0 };  /* LHA7668B official driver handle */

void LHA7668_Platform_Init(void *handle)
{
    lhl_lha7668_ctx_t *dev_ctx = (lhl_lha7668_ctx_t *)handle;
    dev_ctx->io.cs.port = GPIOA;
    dev_ctx->io.cs.pin = GPIO_PIN_4;
}

void LHA7668_Platform_Set(const void *port, const uint32_t pin)
{
    HAL_GPIO_WritePin((GPIO_TypeDef *)port, pin, GPIO_PIN_SET);
}

void LHA7668_Platform_Reset(const void *port, const uint32_t pin)
{
    HAL_GPIO_WritePin((GPIO_TypeDef *)port, pin, GPIO_PIN_RESET);
}

int32_t LHA7668_Platform_ReadWrite(uint8_t *txdata, uint8_t *rxdata, const uint16_t size)
{
    if (HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, size, 100) != HAL_OK) {
        /* SPI 出错（OVR/MODF 等）时中止并复位外设，避免后续传输永久卡在超时上 */
        HAL_SPI_Abort(&hspi1);
        return -1;
    }
    return 0;
}

/* 单颗 LHA7668 三通道(CH0/CH1/CH2)依次转换结果的读取。
 *
 * LHA7668 为 AD7124 兼容芯片，单次模式 + 多通道使能时，
 * 写一次 MODE 即触发 CH0→CH1→CH2 整个序列，每次读 DATA 自动推进下一通道。
 * 因此这里只 Start 一次，连续读 3 次即可取回三个通道。
 *
 * 关键点：
 *  1) STATUS.RDY 位：1=转换中，0=数据就绪(DOUT/RDY 低有效)。
 *     必须等 RDY==0 才能读数据，循环条件为 (RDY==SET) 时继续等。
 *  2) 每包数据用状态字节的 CH_ACTIVE(bit[3:0]) 校验归属通道，按通道归位，
 *     而非按读取顺序假设——彻底杜绝 FX/FY/FZ 相互错位。
 *
 * 返回 0=三通道全部成功；非 0=超时/序列异常(输出不更新) */
static int32_t int_adcData_3ch_core(lhl_lha7668_ctx_t *ctx,
                                    uint32_t *out0, uint32_t *out1, uint32_t *out2)
{
    uint32_t *out[3] = { out0, out1, out2 };
    uint8_t   got    = 0;                 /* 已成功收集的通道位图 */
    uint8_t   ch;
    uint32_t  wait;
    uint8_t   round;

    LHL_LHA7668_Start(ctx, LHA7668_MODE_SINGLE_SHOT);   /* 一次触发 CH0→CH1→CH2 序列 */

    /* 循环读取：正常 3 轮集齐；最多 6 轮容错（序列偶发从非 CH0 起步时多读几包） */
    for (round = 0; round < 6; round++) {
        wait = LHA7668_RDY_WAIT_MAX;
        /* 等本次转换完成：RDY 由 1(转换中) 变 0(数据就绪) */
        while ((LHL_LHA7668_Get_Flag(ctx, LHA7668_STATUS_RDY_FLAG) == LHA7668_SET) && (--wait));
        if (!wait) {
            return -1;                    /* 转换超时/SPI 异常 */
        }
        LHL_LHA7668_Get_Data(ctx);

        /* 用状态字节校验数据归属通道 */
        ch = (uint8_t)(ctx->status & LHA7668_STATUS_CH_ACTIVE_MSK);
        if (ch < 3 && !(got & (1U << ch))) {
            if (out[ch]) {
                *out[ch] = ctx->data;     /* 按通道归位，绝不串位 */
            }
            got |= (1U << ch);
            if (got == 0x07U) {           /* CH0/CH1/CH2 全部集齐 */
                return 0;
            }
        }
        /* 数据不是未收集的通道(重复/序列起点偏移)：丢弃，继续读下一包 */
    }
    return -1;
}

int32_t int_adcData_3ch(uint32_t *fx, uint32_t *fy, uint32_t *fz)
{
    return int_adcData_3ch_core(&lha7668_ctx, fx, fy, fz);
}

/* 单颗 LHA7668 六通道(CH0~CH5)依次转换结果的读取。
 *
 * 与 int_adcData_3ch_core 同理：单次模式一次 Start 触发 CH0→CH1→…→CH5 序列，
 * 每次读 DATA 自动推进下一通道；用 STATUS.CH_ACTIVE 校验归属通道，按通道归位，
 * 而非按读取顺序假设——彻底杜绝 6 个通道相互错位。
 *
 * 返回 0=六通道全部成功；非 0=超时/序列异常(输出不更新) */
static int32_t int_adcData_6ch_core(lhl_lha7668_ctx_t *ctx,
                                    uint32_t *out0, uint32_t *out1, uint32_t *out2,
                                    uint32_t *out3, uint32_t *out4, uint32_t *out5)
{
    uint32_t *out[6] = { out0, out1, out2, out3, out4, out5 };
    uint8_t   got    = 0;                 /* 已成功收集的通道位图 */
    uint8_t   ch;
    uint32_t  wait;
    uint8_t   round;

    LHL_LHA7668_Start(ctx, LHA7668_MODE_SINGLE_SHOT);   /* 一次触发 CH0→…→CH5 序列 */

    /* 循环读取：正常 6 轮集齐；最多 12 轮容错（序列偶发从非 CH0 起步时多读几包） */
    for (round = 0; round < 12; round++) {
        wait = LHA7668_RDY_WAIT_MAX;
        /* 等本次转换完成：RDY 由 1(转换中) 变 0(数据就绪) */
        while ((LHL_LHA7668_Get_Flag(ctx, LHA7668_STATUS_RDY_FLAG) == LHA7668_SET) && (--wait));
        if (!wait) {
            return -1;                    /* 转换超时/SPI 异常 */
        }
        LHL_LHA7668_Get_Data(ctx);

        /* 用状态字节校验数据归属通道 */
        ch = (uint8_t)(ctx->status & LHA7668_STATUS_CH_ACTIVE_MSK);
        if (ch < 6 && !(got & (1U << ch))) {
            if (out[ch]) {
                *out[ch] = ctx->data;     /* 按通道归位，绝不串位 */
            }
            got |= (1U << ch);
            if (got == 0x3FU) {           /* CH0~CH5 全部集齐 */
                return 0;
            }
        }
        /* 数据不是未收集的通道(重复/序列起点偏移)：丢弃，继续读下一包 */
    }
    return -1;
}

int32_t int_adcData_6ch(uint32_t *raw)
{
    return int_adcData_6ch_core(&lha7668_ctx, &raw[0], &raw[1], &raw[2],
                                &raw[3], &raw[4], &raw[5]);
}

float adcData(void)
{
    uint32_t wait = LHA7668_RDY_WAIT_MAX;
    LHL_LHA7668_Start(&lha7668_ctx, LHA7668_MODE_SINGLE_SHOT);
    while ((LHL_LHA7668_Get_Flag(&lha7668_ctx, LHA7668_STATUS_RDY_FLAG) == LHA7668_SET) && (--wait));
    if (wait) {
        LHL_LHA7668_Get_Data(&lha7668_ctx);
    }
    return LHL_LHA7668_Get_mVoltage(lha7668_ctx.data, LHA7668_BIPOLAR, LHA7668_PGA_X128, 2500);
}

void lha7668_init(uint8_t rate)
{
    LHA7668_Platform_Init(&lha7668_ctx);
    LHL_LHA7668_Init(&lha7668_ctx);
    LHL_LHA7668_Reset(&lha7668_ctx);

    uint8_t whoamI = 0xFF;
    adc_init_count = 0;
    while (1) {
        whoamI = LHL_LHA7668_Get_ID(&lha7668_ctx);
        if (whoamI == LHA7668B_8) {
            break;
        } else {
            adc_init_count++;
            uart_debug("ADC_1 Init Failed\r\n");
            original_send(&adc_init_count, 1);
        }
    }
    LHL_LHA7668_Stop(&lha7668_ctx);

    lha7668_ctx.CHANNEL.ENABLE = LHA7668_ENABLE;
    lha7668_ctx.CHANNEL.SETUP = LHA7668_SETUP_0;
    lha7668_ctx.CHANNEL.AINP = LHA7668_AIN0;
    lha7668_ctx.CHANNEL.AINM = LHA7668_AIN1;
    LHL_LHA7668_Set_Channel(&lha7668_ctx, LHA7668_CHANNEL_0);
    lha7668_ctx.CHANNEL.ENABLE = LHA7668_ENABLE;
    lha7668_ctx.CHANNEL.SETUP = LHA7668_SETUP_0;
    lha7668_ctx.CHANNEL.AINP = LHA7668_AIN2;
    lha7668_ctx.CHANNEL.AINM = LHA7668_AIN3;
    LHL_LHA7668_Set_Channel(&lha7668_ctx, LHA7668_CHANNEL_1);
    lha7668_ctx.CHANNEL.ENABLE = LHA7668_ENABLE;
    lha7668_ctx.CHANNEL.SETUP = LHA7668_SETUP_0;
    lha7668_ctx.CHANNEL.AINP = LHA7668_AIN4;
    lha7668_ctx.CHANNEL.AINM = LHA7668_AIN5;
    LHL_LHA7668_Set_Channel(&lha7668_ctx, LHA7668_CHANNEL_2);
    lha7668_ctx.CHANNEL.ENABLE = LHA7668_ENABLE;
    lha7668_ctx.CHANNEL.SETUP = LHA7668_SETUP_0;
    lha7668_ctx.CHANNEL.AINP = LHA7668_AIN6;
    lha7668_ctx.CHANNEL.AINM = LHA7668_AIN7;
    LHL_LHA7668_Set_Channel(&lha7668_ctx, LHA7668_CHANNEL_3);
    lha7668_ctx.CHANNEL.ENABLE = LHA7668_ENABLE;
    lha7668_ctx.CHANNEL.SETUP = LHA7668_SETUP_0;
    lha7668_ctx.CHANNEL.AINP = LHA7668_AIN8;
    lha7668_ctx.CHANNEL.AINM = LHA7668_AIN9;
    LHL_LHA7668_Set_Channel(&lha7668_ctx, LHA7668_CHANNEL_4);
    lha7668_ctx.CHANNEL.ENABLE = LHA7668_ENABLE;
    lha7668_ctx.CHANNEL.SETUP = LHA7668_SETUP_0;
    lha7668_ctx.CHANNEL.AINP = LHA7668_AIN10;
    lha7668_ctx.CHANNEL.AINM = LHA7668_AIN11;
    LHL_LHA7668_Set_Channel(&lha7668_ctx, LHA7668_CHANNEL_5);

    lha7668_ctx.SETUP.BIPOLAR = LHA7668_BIPOLAR;
    lha7668_ctx.SETUP.AIN_BUFP = LHA7668_ENABLE;
    lha7668_ctx.SETUP.AIN_BUFM = LHA7668_ENABLE;
    lha7668_ctx.SETUP.REF_SEL = LHA7668_REF_REFIN1;
    lha7668_ctx.SETUP.REF_BUFP = LHA7668_DISABLE;
    lha7668_ctx.SETUP.REF_BUFM = LHA7668_DISABLE;
    lha7668_ctx.SETUP.PGA = LHA7668_PGA_X128;
    lha7668_ctx.SETUP.FILTER = LHA7668_FILTER_SINC3;
    lha7668_ctx.SETUP.FS = rate;
    LHL_LHA7668_Set_Setup(&lha7668_ctx, LHA7668_SETUP_0);
    LHL_LHA7668_Set_Setup(&lha7668_ctx, LHA7668_SETUP_1);
    LHL_LHA7668_Set_Setup(&lha7668_ctx, LHA7668_SETUP_2);
    LHL_LHA7668_Set_Setup(&lha7668_ctx, LHA7668_SETUP_3);
    LHL_LHA7668_Set_Setup(&lha7668_ctx, LHA7668_SETUP_4);
    LHL_LHA7668_Set_Setup(&lha7668_ctx, LHA7668_SETUP_5);
    lha7668_ctx.ADC_CTRL.POWER_MODE = LHA7668_FULL_POWER;
    lha7668_ctx.ADC_CTRL.CLK_SEL = LHA7668_CLK_INTL;
    lha7668_ctx.ADC_CTRL.DATA_STATUS = LHA7668_ENABLE;
    lha7668_ctx.ADC_CTRL.CS_EN = LHA7668_DISABLE;
    lha7668_ctx.ADC_CTRL.DOUT_RDY_DEL = LHA7668_DISABLE;
    LHL_LHA7668_Set_ADC(&lha7668_ctx);

    uart_debug("ADC_1 completed init OK\r\n");
}
