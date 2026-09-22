/******************************************************************************
 * 版权所有：苏州领慧立芯科技有限公司
 * Copyright (c) 2020-2025 Suzhou Legendsemi Technology Co., Ltd.
 ******************************************************************************
 * All rights reserved. Distributed under MIT license.
 * The file is encoded in UTF-8 without signature.
 ******************************************************************************/
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LHA7668_H_
#define __LHA7668_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>
#include <math.h>

#define LHA7668_Little_Endian       1   // 大小端架构：1->Little Endian; 0->Big Endian
#define LHA7668_System_Core_Clock   200 // 主控核心频率（MHz）：GD32F330->84MHz; STM32H750->400MHz

/******************************************************************************
 * 1. LHA7668 通用寄存器声明
 ******************************************************************************/

/* 1.1 Register Address ------------------------------------------------------*/
#define LHA7668_COMM            0x00
#define LHA7668_STATUS          0x00
#define LHA7668_ADC_CTRL        0x01
#define LHA7668_DATA            0x02
#define LHA7668_IO_CTRL1        0x03
#define LHA7668_IO_CTRL2        0x04
#define LHA7668_ID        	    0x05
#define LHA7668_ERR       	    0x06
#define LHA7668_ERREN     	    0x07
#define LHA7668_MCLK		    0x08
#define LHA7668_CHANNEL_0       0x09
#define LHA7668_CHANNEL_1       0x0A
#define LHA7668_CHANNEL_2       0x0B
#define LHA7668_CHANNEL_3       0x0C
#define LHA7668_CHANNEL_4       0x0D
#define LHA7668_CHANNEL_5       0x0E
#define LHA7668_CHANNEL_6       0x0F
#define LHA7668_CHANNEL_7       0x10
#define LHA7668_CHANNEL_8       0x11
#define LHA7668_CHANNEL_9       0x12
#define LHA7668_CHANNEL_10      0x13
#define LHA7668_CHANNEL_11      0x14
#define LHA7668_CHANNEL_12      0x15
#define LHA7668_CHANNEL_13      0x16
#define LHA7668_CHANNEL_14      0x17
#define LHA7668_CHANNEL_15      0x18
#define LHA7668_CONFIG_0   	    0x19
#define LHA7668_CONFIG_1      	0x1A
#define LHA7668_CONFIG_2      	0x1B
#define LHA7668_CONFIG_3      	0x1C
#define LHA7668_CONFIG_4      	0x1D
#define LHA7668_CONFIG_5      	0x1E
#define LHA7668_CONFIG_6      	0x1F
#define LHA7668_CONFIG_7      	0x20
#define LHA7668_FILTER_0     	0x21
#define LHA7668_FILTER_1     	0x22
#define LHA7668_FILTER_2     	0x23
#define LHA7668_FILTER_3     	0x24
#define LHA7668_FILTER_4     	0x25
#define LHA7668_FILTER_5     	0x26
#define LHA7668_FILTER_6     	0x27
#define LHA7668_FILTER_7     	0x28
#define LHA7668_OFFSET_0     	0x29
#define LHA7668_OFFSET_1     	0x2A
#define LHA7668_OFFSET_2     	0x2B
#define LHA7668_OFFSET_3     	0x2C
#define LHA7668_OFFSET_4     	0x2D
#define LHA7668_OFFSET_5     	0x2E
#define LHA7668_OFFSET_6     	0x2F
#define LHA7668_OFFSET_7     	0x30
#define LHA7668_GAIN_0          0x31
#define LHA7668_GAIN_1     	    0x32
#define LHA7668_GAIN_2     	    0x33
#define LHA7668_GAIN_3     	    0x34
#define LHA7668_GAIN_4     	    0x35
#define LHA7668_GAIN_5     	    0x36
#define LHA7668_GAIN_6     	    0x37
#define LHA7668_GAIN_7     	    0x38

/* 1.2 Register Size ---------------------------------------------------------*/
#define LHA7668_COMM_SIZE       0x01
#define LHA7668_STATUS_SIZE     0x01
#define LHA7668_ADC_CTRL_SIZE   0x02
#define LHA7668_DATA_SIZE       0x03
#define LHA7668_IO_CTRL1_SIZE   0x03
#define LHA7668_IO_CTRL2_SIZE   0x02
#define LHA7668_ID_SIZE         0x01
#define LHA7668_ERR_SIZE        0x03
#define LHA7668_ERREN_SIZE      0x03
#define LHA7668_MCLK_SIZE       0x01
#define LHA7668_CHANNEL_SIZE    0x02
#define LHA7668_CONFIG_SIZE     0x02
#define LHA7668_FILTER_SIZE     0x03
#define LHA7668_OFFSET_SIZE     0x03
#define LHA7668_GAIN_SIZE       0x03

/* 1.3 Register Bits Definition ----------------------------------------------*/
/* Communication Register bits */
#define LHA7668_COMM_WEN_POS                    (7U)
#define LHA7668_COMM_WEN_MSK                    (1UL << LHA7668_COMM_WEN_POS)
#define LHA7668_COMM_R_W_POS                    (6U)
#define LHA7668_COMM_R_W_MSK                    (1UL << LHA7668_COMM_R_W_POS)
#define LHA7668_COMM_RS_POS                     (0U)
#define LHA7668_COMM_RS_MSK                     (0x3FUL << LHA7668_COMM_RS_POS)

/* Status Register bits */
#define LHA7668_STATUS_RDY_POS                  (7U)
#define LHA7668_STATUS_RDY_MSK                  (1UL << LHA7668_STATUS_RDY_POS)
#define LHA7668_STATUS_ERROR_FLAG_POS           (6U)
#define LHA7668_STATUS_ERROR_FLAG_MSK           (1UL << LHA7668_STATUS_ERROR_FLAG_POS)
#define LHA7668_STATUS_POR_FLAG_POS             (4U)
#define LHA7668_STATUS_POR_FLAG_MSK             (1UL << LHA7668_STATUS_POR_FLAG_POS)
#define LHA7668_STATUS_CH_ACTIVE_POS            (0U)
#define LHA7668_STATUS_CH_ACTIVE_MSK	        (0xFUL << LHA7668_STATUS_CH_ACTIVE_POS)

/* ADC_Control Register bits */
#define LHA7668_ADC_CTRL_DOUT_RDY_DEL_POS       (12U)
#define LHA7668_ADC_CTRL_DOUT_RDY_DEL_MSK       (1UL << LHA7668_ADC_CTRL_DOUT_RDY_DEL_POS)
#define LHA7668_ADC_CTRL_CONT_READ_POS          (11U)
#define LHA7668_ADC_CTRL_CONT_READ_MSK          (1UL << LHA7668_ADC_CTRL_CONT_READ_POS)
#define LHA7668_ADC_CTRL_DATA_STATUS_POS        (10U)
#define LHA7668_ADC_CTRL_DATA_STATUS_MSK        (1UL << LHA7668_ADC_CTRL_DATA_STATUS_POS)
#define LHA7668_ADC_CTRL_CS_EN_POS              (9U)
#define LHA7668_ADC_CTRL_CS_EN_MSK              (1UL << LHA7668_ADC_CTRL_CS_EN_POS)
#define LHA7668_ADC_CTRL_REF_EN_POS             (8U)
#define LHA7668_ADC_CTRL_REF_EN_MSK             (1UL << LHA7668_ADC_CTRL_REF_EN_POS)
#define LHA7668_ADC_CTRL_POWER_MODE_POS         (6U)
#define LHA7668_ADC_CTRL_POWER_MODE_MSK         (0x3UL << LHA7668_ADC_CTRL_POWER_MODE_POS)
#define LHA7668_ADC_CTRL_MODE_POS               (2U)
#define LHA7668_ADC_CTRL_MODE_MSK               (0xFUL << LHA7668_ADC_CTRL_MODE_POS)
#define LHA7668_ADC_CTRL_CLK_SEL_POS            (0U)
#define LHA7668_ADC_CTRL_CLK_SEL_MSK            (0x3UL << LHA7668_ADC_CTRL_CLK_SEL_POS)

/* IO_Control_1 LHA7668 Register bits */
#define LHA7668_IO_CTRL1_GPIO_DAT4_POS		    (23U)
#define LHA7668_IO_CTRL1_GPIO_DAT4_MSK		    (1UL << LHA7668_IO_CTRL1_GPIO_DAT4_POS)
#define LHA7668_IO_CTRL1_GPIO_DAT3_POS		    (22U)
#define LHA7668_IO_CTRL1_GPIO_DAT3_MSK		    (1UL << LHA7668_IO_CTRL1_GPIO_DAT3_POS)
#define LHA7668_IO_CTRL1_GPIO_DAT2_POS		    (21U)
#define LHA7668_IO_CTRL1_GPIO_DAT2_MSK		    (1UL << LHA7668_IO_CTRL1_GPIO_DAT2_POS)
#define LHA7668_IO_CTRL1_GPIO_DAT1_POS		    (20U)
#define LHA7668_IO_CTRL1_GPIO_DAT1_MSK		    (1UL << LHA7668_IO_CTRL1_GPIO_DAT1_POS)
#define LHA7668_IO_CTRL1_GPIO_CTRL4_POS		    (19U)
#define LHA7668_IO_CTRL1_GPIO_CTRL4_MSK	        (1UL << LHA7668_IO_CTRL1_GPIO_CTRL4_POS)
#define LHA7668_IO_CTRL1_GPIO_CTRL3_POS	        (18U)
#define LHA7668_IO_CTRL1_GPIO_CTRL3_MSK		    (1UL << LHA7668_IO_CTRL1_GPIO_CTRL3_POS)
#define LHA7668_IO_CTRL1_GPIO_CTRL2_POS		    (17U)
#define LHA7668_IO_CTRL1_GPIO_CTRL2_MSK		    (1UL << LHA7668_IO_CTRL1_GPIO_CTRL2_POS)
#define LHA7668_IO_CTRL1_GPIO_CTRL1_POS		    (16U)
#define LHA7668_IO_CTRL1_GPIO_CTRL1_MSK		    (1UL << LHA7668_IO_CTRL1_GPIO_CTRL1_POS)
#define LHA7668_IO_CTRL1_PDSW_POS          	    (15U)
#define LHA7668_IO_CTRL1_PDSW_MSK         	    (1UL << LHA7668_IO_CTRL1_PDSW_POS)
#define LHA7668_IO_CTRL1_IOUT1_POS      	    (11U)
#define LHA7668_IO_CTRL1_IOUT1_MSK      	    (0x7UL << LHA7668_IO_CTRL1_IOUT1_POS)
#define LHA7668_IO_CTRL1_IOUT0_POS     	        (8U)
#define LHA7668_IO_CTRL1_IOUT0_MSK     	        (0x7UL << LHA7668_IO_CTRL1_IOUT0_POS)
#define LHA7668_IO_CTRL1_IOUT1_CH_POS   	    (4U)
#define LHA7668_IO_CTRL1_IOUT1_CH_MSK   	    (0xFUL << LHA7668_IO_CTRL1_IOUT1_CH_POS)
#define LHA7668_IO_CTRL1_IOUT0_CH_POS  	        (0U)
#define LHA7668_IO_CTRL1_IOUT0_CH_MSK   	    (0xFUL << LHA7668_IO_CTRL1_IOUT0_CH_POS)

/* IO_Control_1 LHA7668-4 Register bits */
#define LHA7668_4_IO_CTRL1_GPIO_DAT2_POS        (23U)
#define LHA7668_4_IO_CTRL1_GPIO_DAT2_MSK        (1UL << LHA7668_4_IO_CTRL1_GPIO_DAT2_POS)
#define LHA7668_4_IO_CTRL1_GPIO_DAT1_POS        (22U)
#define LHA7668_4_IO_CTRL1_GPIO_DAT1_MSK        (1UL << LHA7668_4_IO_CTRL1_GPIO_DAT1_POS)
#define LHA7668_4_IO_CTRL1_GPIO_CTRL2_POS       (19U)
#define LHA7668_4_IO_CTRL1_GPIO_CTRL2_MSK       (1UL << LHA7668_4_IO_CTRL1_GPIO_CTRL2_POS)
#define LHA7668_4_IO_CTRL1_GPIO_CTRL1_POS       (18U)
#define LHA7668_4_IO_CTRL1_GPIO_CTRL1_SMK       (1UL << LHA7668_4_IO_CTRL1_GPIO_CTRL1_POS)

/* IO_Control_2 LHA7668 Register bits */
#define LHA7668_IO_CTRL2_VBIAS15_POS	        (15U)
#define LHA7668_IO_CTRL2_VBIAS15_MSK	        (1UL << LHA7668_IO_CTRL2_VBIAS15_POS)
#define LHA7668_IO_CTRL2_VBIAS14_POS	        (14U)
#define LHA7668_IO_CTRL2_VBIAS14_MSK	        (1UL << LHA7668_IO_CTRL2_VBIAS14_POS)
#define LHA7668_IO_CTRL2_VBIAS13_POS	        (13U)
#define LHA7668_IO_CTRL2_VBIAS13_MSK	        (1UL << LHA7668_IO_CTRL2_VBIAS13_POS)
#define LHA7668_IO_CTRL2_VBIAS12_POS	        (12U)
#define LHA7668_IO_CTRL2_VBIAS12_MSK	        (1UL << LHA7668_IO_CTRL2_VBIAS12_POS)
#define LHA7668_IO_CTRL2_VBIAS11_POS	        (11U)
#define LHA7668_IO_CTRL2_VBIAS11_MSK	        (1UL << LHA7668_IO_CTRL2_VBIAS11_POS)
#define LHA7668_IO_CTRL2_VBIAS10_POS	        (10U)
#define LHA7668_IO_CTRL2_VBIAS10_MSK	        (1UL << LHA7668_IO_CTRL2_VBIAS10_POS)
#define LHA7668_IO_CTRL2_VBIAS9_POS	            (9U)
#define LHA7668_IO_CTRL2_VBIAS9_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS9_POS)
#define LHA7668_IO_CTRL2_VBIAS8_POS	            (8U)
#define LHA7668_IO_CTRL2_VBIAS8_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS8_POS)
#define LHA7668_IO_CTRL2_VBIAS7_POS	            (7U)
#define LHA7668_IO_CTRL2_VBIAS7_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS7_POS)
#define LHA7668_IO_CTRL2_VBIAS6_POS	            (6U)
#define LHA7668_IO_CTRL2_VBIAS6_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS6_POS)
#define LHA7668_IO_CTRL2_VBIAS5_POS	            (5U)
#define LHA7668_IO_CTRL2_VBIAS5_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS5_POS)
#define LHA7668_IO_CTRL2_VBIAS4_POS	            (4U)
#define LHA7668_IO_CTRL2_VBIAS4_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS4_POS)
#define LHA7668_IO_CTRL2_VBIAS3_POS	            (3U)
#define LHA7668_IO_CTRL2_VBIAS3_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS3_POS)
#define LHA7668_IO_CTRL2_VBIAS2_POS	            (2U)
#define LHA7668_IO_CTRL2_VBIAS2_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS2_POS)
#define LHA7668_IO_CTRL2_VBIAS1_POS	            (1U)
#define LHA7668_IO_CTRL2_VBIAS1_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS1_POS)
#define LHA7668_IO_CTRL2_VBIAS0_POS	            (0U)
#define LHA7668_IO_CTRL2_VBIAS0_MSK	            (1UL << LHA7668_IO_CTRL2_VBIAS0_POS)

/* IO_Control_2 LHA7668-4 Register bits */
#define LHA7668_4_IO_CTRL2_VBIAS7_POS	        (15U)
#define LHA7668_4_IO_CTRL2_VBIAS7_MSK	        (1UL << LHA7668_4_IO_CTRL2_VBIAS7_POS)
#define LHA7668_4_IO_CTRL2_VBIAS6_POS	        (14U)
#define LHA7668_4_IO_CTRL2_VBIAS6_MSK	        (1UL << LHA7668_4_IO_CTRL2_VBIAS6_POS)
#define LHA7668_4_IO_CTRL2_VBIAS5_POS	        (11U)
#define LHA7668_4_IO_CTRL2_VBIAS5_MSK	        (1UL << LHA7668_4_IO_CTRL2_VBIAS5_POS)
#define LHA7668_4_IO_CTRL2_VBIAS4_POS	        (10U)
#define LHA7668_4_IO_CTRL2_VBIAS4_MSK	        (1UL << LHA7668_4_IO_CTRL2_VBIAS4_POS)
#define LHA7668_4_IO_CTRL2_VBIAS3_POS	        (5U)
#define LHA7668_4_IO_CTRL2_VBIAS3_MSK	        (1UL << LHA7668_4_IO_CTRL2_VBIAS3_POS)
#define LHA7668_4_IO_CTRL2_VBIAS2_POS	        (4U)
#define LHA7668_4_IO_CTRL2_VBIAS2_MSK	        (1UL << LHA7668_4_IO_CTRL2_VBIAS2_POS)
#define LHA7668_4_IO_CTRL2_VBIAS1_POS	        (1U)
#define LHA7668_4_IO_CTRL2_VBIAS1_MSK	        (1UL << LHA7668_4_IO_CTRL2_VBIAS1_POS)
#define LHA7668_4_IO_CTRL2_VBIAS0_POS	        (0U)
#define LHA7668_4_IO_CTRL2_VBIAS0_MSK	        (1UL << LHA7668_4_IO_CTRL2_VBIAS0_POS)

/* ID Register bits */
#define LHA7668_ID_DEVICE_ID_POS		        (4U)
#define LHA7668_ID_DEVICE_ID_MSK		        (0xFUL << LHA7668_ID_DEVICE_ID_POS)
#define LHA7668_ID_SILICON_REV_POS		        (0U)
#define LHA7668_ID_SILICON_REV_MSK		        (0xFUL << LHA7668_ID_SILICON_REV_POS)

/* Error Register bits */
#define LHA7668_ERR_LDO_CAP_ERR_POS			    (19U)
#define LHA7668_ERR_LDO_CAP_ERR_MSK			    (1UL << LHA7668_ERR_LDO_CAP_ERR_POS)
#define LHA7668_ERR_ADC_CAL_ERR_POS        	    (18U)
#define LHA7668_ERR_ADC_CAL_ERR_MSK        	    (1UL << LHA7668_ERR_ADC_CAL_ERR_POS)
#define LHA7668_ERR_ADC_CONV_ERR_POS       	    (17U)
#define LHA7668_ERR_ADC_CONV_ERR_MSK       	    (1UL << LHA7668_ERR_ADC_CONV_ERR_POS)
#define LHA7668_ERR_ADC_SAT_ERR_POS        	    (16U)
#define LHA7668_ERR_ADC_SAT_ERR_MSK        	    (1UL << LHA7668_ERR_ADC_SAT_ERR_POS)
#define LHA7668_ERR_AINP_OV_ERR_POS        	    (15U)
#define LHA7668_ERR_AINP_OV_ERR_MSK        	    (1UL << LHA7668_ERR_AINP_OV_ERR_POS)
#define LHA7668_ERR_AINP_UV_ERR_POS        	    (14U)
#define LHA7668_ERR_AINP_UV_ERR_MSK        	    (1UL << LHA7668_ERR_AINP_UV_ERR_POS)
#define LHA7668_ERR_AINM_OV_ERR_POS        	    (13U)
#define LHA7668_ERR_AINM_OV_ERR_MSK        	    (1UL << LHA7668_ERR_AINM_OV_ERR_POS)
#define LHA7668_ERR_AINM_UV_ERR_POS        	    (12U)
#define LHA7668_ERR_AINM_UV_ERR_MSK        	    (1UL << LHA7668_ERR_AINM_UV_ERR_POS)
#define LHA7668_ERR_REF_DET_ERR_POS        	    (11U)
#define LHA7668_ERR_REF_DET_ERR_MSK        	    (1UL << LHA7668_ERR_REF_DET_ERR_POS)
#define LHA7668_ERR_DLDO_PSM_ERR_POS       	    (9U)
#define LHA7668_ERR_DLDO_PSM_ERR_MSK       	    (1UL << LHA7668_ERR_DLDO_PSM_ERR_POS)
#define LHA7668_ERR_ALDO_PSM_ERR_POS       	    (7U)
#define LHA7668_ERR_ALDO_PSM_ERR_MSK       	    (1UL << LHA7668_ERR_ALDO_PSM_ERR_POS)
#define LHA7668_ERR_SPI_IGNORE_ERR_POS     	    (6U)
#define LHA7668_ERR_SPI_IGNORE_ERR_MSK     	    (1UL << LHA7668_ERR_SPI_IGNORE_ERR_POS)
#define LHA7668_ERR_SPI_SLCK_CNT_ERR_POS   	    (5U)
#define LHA7668_ERR_SPI_SLCK_CNT_ERR_MSK   	    (1UL << LHA7668_ERR_SPI_SLCK_CNT_ERR_POS)
#define LHA7668_ERR_SPI_READ_ERR_POS       	    (4U)
#define LHA7668_ERR_SPI_READ_ERR_MSK       	    (1UL << LHA7668_ERR_SPI_READ_ERR_POS)
#define LHA7668_ERR_SPI_WRITE_ERR_POS      	    (3U)
#define LHA7668_ERR_SPI_WRITE_ERR_MSK      	    (1UL << LHA7668_ERR_SPI_WRITE_ERR_POS)
#define LHA7668_ERR_SPI_CRC_ERR_POS        	    (2U)
#define LHA7668_ERR_SPI_CRC_ERR_MSK        	    (1UL << LHA7668_ERR_SPI_CRC_ERR_POS)
#define LHA7668_ERR_MM_CRC_ERR_POS         	    (1U)
#define LHA7668_ERR_MM_CRC_ERR_MSK         	    (1UL << LHA7668_ERR_MM_CRC_ERR_POS)
#define LHA7668_ERR_ROM_CRC_ERR_POS        	    (0U)
#define LHA7668_ERR_ROM_CRC_ERR_MSK        	    (1UL << LHA7668_ERR_ROM_CRC_ERR_POS)

/* Error_En Register bits */
#define LHA7668_ERREN_MCLK_CNT_EN_POS			(22U)
#define LHA7668_ERREN_MCLK_CNT_EN_MSK			(1UL << LHA7668_ERREN_MCLK_CNT_EN_POS)
#define LHA7668_ERREN_LDO_CAP_CHK_TEST_EN_POS   (21U)
#define LHA7668_ERREN_LDO_CAP_CHK_TEST_EN_MSK   (1UL << LHA7668_ERREN_LDO_CAP_CHK_TEST_EN_POS)
#define LHA7668_ERREN_LDO_CAP_CHK_POS           (19U)
#define LHA7668_ERREN_LDO_CAP_CHK_MSK           (0x3UL << LHA7668_ERREN_LDO_CAP_CHK_POS)
#define LHA7668_ERREN_ADC_CAL_ERR_EN_POS        (18U)
#define LHA7668_ERREN_ADC_CAL_ERR_EN_MSK        (1UL << LHA7668_ERREN_ADC_CAL_ERR_EN_POS)
#define LHA7668_ERREN_ADC_CONV_ERR_EN_POS       (17U)
#define LHA7668_ERREN_ADC_CONV_ERR_EN_MSK       (1UL << LHA7668_ERREN_ADC_CONV_ERR_EN_POS)
#define LHA7668_ERREN_ADC_SAT_ERR_EN_POS        (16U)
#define LHA7668_ERREN_ADC_SAT_ERR_EN_MSK        (1UL << LHA7668_ERREN_ADC_SAT_ERR_EN_POS)
#define LHA7668_ERREN_AINP_OV_ERR_EN_POS        (15U)
#define LHA7668_ERREN_AINP_OV_ERR_EN_MSK        (1UL << LHA7668_ERREN_AINP_OV_ERR_EN_POS)
#define LHA7668_ERREN_AINP_UV_ERR_EN_POS        (14U)
#define LHA7668_ERREN_AINP_UV_ERR_EN_MSK        (1UL << LHA7668_ERREN_AINP_UV_ERR_EN_POS)
#define LHA7668_ERREN_AINM_OV_ERR_EN_POS        (13U)
#define LHA7668_ERREN_AINM_OV_ERR_EN_MSK        (1UL << LHA7668_ERREN_AINM_OV_ERR_EN_POS)
#define LHA7668_ERREN_AINM_UV_ERR_EN_POS        (12U)
#define LHA7668_ERREN_AINM_UV_ERR_EN_MSK        (1UL << LHA7668_ERREN_AINM_UV_ERR_EN_POS)
#define LHA7668_ERREN_REF_DET_ERR_EN_POS        (11U)
#define LHA7668_ERREN_REF_DET_ERR_EN_MSK        (1UL << LHA7668_ERREN_REF_DET_ERR_EN_POS)
#define LHA7668_ERREN_DLDO_PSM_TRIP_TEST_EN_POS (10U)
#define LHA7668_ERREN_DLDO_PSM_TRIP_TEST_EN_MSK (1UL << LHA7668_ERREN_DLDO_PSM_TRIP_TEST_EN_POS)
#define LHA7668_ERREN_DLDO_PSM_ERR_ERR_POS      (9U)
#define LHA7668_ERREN_DLDO_PSM_ERR_ERR_MSK      (1UL << LHA7668_ERREN_DLDO_PSM_ERR_ERR_POS)
#define LHA7668_ERREN_ALDO_PSM_TRIP_TEST_EN_POS (8U)
#define LHA7668_ERREN_ALDO_PSM_TRIP_TEST_EN_MSK (1UL << LHA7668_ERREN_ALDO_PSM_TRIP_TEST_EN_POS)
#define LHA7668_ERREN_ALDO_PSM_ERR_EN_POS       (7U)
#define LHA7668_ERREN_ALDO_PSM_ERR_EN_MSK       (1UL << LHA7668_ERREN_ALDO_PSM_ERR_EN_POS)
#define LHA7668_ERREN_SPI_IGNORE_ERR_EN_POS     (6U)
#define LHA7668_ERREN_SPI_IGNORE_ERR_EN_MSK     (1UL << LHA7668_ERREN_SPI_IGNORE_ERR_EN_POS)
#define LHA7668_ERREN_SPI_SCLK_CNT_ERR_EN_POS   (5U)
#define LHA7668_ERREN_SPI_SCLK_CNT_ERR_EN_MSK   (1UL << LHA7668_ERREN_SPI_SCLK_CNT_ERR_EN_POS)
#define LHA7668_ERREN_SPI_READ_ERR_EN_POS       (4U)
#define LHA7668_ERREN_SPI_READ_ERR_EN_MSK       (1UL << LHA7668_ERREN_SPI_READ_ERR_EN_POS)
#define LHA7668_ERREN_SPI_WRITE_ERR_EN_POS      (3U)
#define LHA7668_ERREN_SPI_WRITE_ERR_EN_MSK      (1UL << LHA7668_ERREN_SPI_WRITE_ERR_EN_POS)
#define LHA7668_ERREN_SPI_CRC_ERR_EN_POS        (2U)
#define LHA7668_ERREN_SPI_CRC_ERR_EN_MSK        (1UL << LHA7668_ERREN_SPI_CRC_ERR_EN_POS)
#define LHA7668_ERREN_MM_CRC_ERR_EN_POS         (1U)
#define LHA7668_ERREN_MM_CRC_ERR_EN_MSK         (1UL << LHA7668_ERREN_MM_CRC_ERR_EN_POS)
#define LHA7668_ERREN_ROM_CRC_ERR_EN_POS        (0U)
#define LHA7668_ERREN_ROM_CRC_ERR_EN_MSK        (1UL << LHA7668_ERREN_ROM_CRC_ERR_EN_POS)

/* Channel 0-15 Registers bits */
#define LHA7668_CHANNEL_ENABLE_POS			    (15U)
#define LHA7668_CHANNEL_ENABLE_MSK		        (1UL << LHA7668_CHANNEL_ENABLE_POS)
#define LHA7668_CHANNEL_SETUP_POS			    (12U)
#define LHA7668_CHANNEL_SETUP_MSK				(0x7UL << LHA7668_CHANNEL_SETUP_POS)
#define LHA7668_CHANNEL_AINP_POS      		    (5U)
#define LHA7668_CHANNEL_AINP_MSK      		    (0x1FUL << LHA7668_CHANNEL_AINP_POS)
#define LHA7668_CHANNEL_AINM_POS      		    (0U)
#define LHA7668_CHANNEL_AINM_MSK      		    (0x1FUL << LHA7668_CHANNEL_AINM_POS)

/* Configuration 0-7 Registers bits */
#define LHA7668_CONFIG_BIPOLAR_POS     			(11U)
#define LHA7668_CONFIG_BIPOLAR_MSK     			(1UL << LHA7668_CONFIG_BIPOLAR_POS)
#define LHA7668_CONFIG_BURNOUT_POS  			(9U)
#define LHA7668_CONFIG_BURNOUT_MSK  			(0x3UL << LHA7668_CONFIG_BURNOUT_POS)
#define LHA7668_CONFIG_REF_BUFP_POS    			(8U)
#define LHA7668_CONFIG_REF_BUFP_MSK    			(1UL << LHA7668_CONFIG_REF_BUFP_POS)
#define LHA7668_CONFIG_REF_BUFM_POS    			(7U)
#define LHA7668_CONFIG_REF_BUFM_MSK    			(1UL << LHA7668_CONFIG_REF_BUFM_POS)
#define LHA7668_CONFIG_AIN_BUFP_POS    			(6U)
#define LHA7668_CONFIG_AIN_BUFP_MSK    			(1UL << LHA7668_CONFIG_AIN_BUFP_POS)
#define LHA7668_CONFIG_AIN_BUFM_POS   			(5U)
#define LHA7668_CONFIG_AIN_BUFM_MSK   			(1UL << LHA7668_CONFIG_AIN_BUFM_POS)
#define LHA7668_CONFIG_REF_SEL_POS  			(3U)
#define LHA7668_CONFIG_REF_SEL_MSK  			(0x3UL << LHA7668_CONFIG_REF_SEL_POS)
#define LHA7668_CONFIG_PGA_POS      			(0U)
#define LHA7668_CONFIG_PGA_MSK      			(0x7UL << LHA7668_CONFIG_PGA_POS)

/* Filter 0-7 Register bits */
#define LHA7668_FILTER_FILTER_POS        		(21U)
#define LHA7668_FILTER_FILTER_MSK         		(0x7UL << LHA7668_FILTER_FILTER_POS)
#define LHA7668_FILTER_REJ60_POS                (20U)
#define LHA7668_FILTER_REJ60_MSK                (1UL << LHA7668_FILTER_REJ60_POS)
#define LHA7668_FILTER_POST_FILTER_POS    		(17U)
#define LHA7668_FILTER_POST_FILTER_MSK    		(0x7UL << LHA7668_FILTER_POST_FILTER_POS)
#define LHA7668_FILTER_SINGLE_CYCLE_POS         (16U)
#define LHA7668_FILTER_SINGLE_CYCLE_MSK         (1UL << LHA7668_FILTER_SINGLE_CYCLE_POS)
#define LHA7668_FILTER_FS_POS             		(0U)
#define LHA7668_FILTER_FS_MSK             		(0x7FFUL << LHA7668_FILTER_FS_POS)

/* 1.4 LHA7668 Constants Definition ------------------------------------------*/
#define LHA7668_8               0x14        /* Device ID */
#define LHA7668_4               0x4
#define LHA7668B_8              0x28
#define LHA7668B_4              0x14

#define LHA7668_RESET_COUNT	    8			/* 复位脉冲数量 8 x 8 = 64 */
#define LHA7668_BYTE_1          0xFF        /* Dummy Byte 1 */
#define LHA7668_BYTE_0          0x00        /* Dummy Byte 0 */

#define LHA7668_CRC8_POLY       0x07        /* x8 + x2 + x + 1 */

/******************************************************************************
 * 2. LHA7668 驱动库成员声明
 ******************************************************************************/
/* 编译器特性检测 */
#if defined(__GNUC__) || defined(__clang__)
    #define __LHA7668_WEAK __attribute__((weak))
    #define __LHA7668_ALIGN_4 __attribute__((aligned(4)))
    #define __LHA7668_NOTHING __asm__ volatile("")
#elif defined(__ICCARM__) || defined(__IAR_SYSTEMS_ICC__)
    #define __LHA7668_WEAK __weak
    #define __LHA7668_ALIGN_4 _Pragma("data_alignment=4")
    #define __LHA7668_NOTHING  __asm volatile ("")
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
    #define __LHA7668_WEAK __weak
    #define __LHA7668_ALIGN_4 __align(4)
    #define __LHA7668_NOTHING __asm volatile ("")
#else
    #error "Unsupported compiler!!!"
#endif

/* 2.1 LHA7668 寄存器枚举定义 ------------------------------------------------*/
typedef enum {
    LHA7668_RESET                   = 0U, 
    LHA7668_SET                     = 1U,
} lha7668_flag_status;

typedef enum {
    LHA7668_DISABLE                 = 0U, 
    LHA7668_ENABLE                  = 1U,
} lha7668_functional_state;

typedef enum {
    LHA7668_LOW_POWER               = 0x00U,
    LHA7668_MID_POWER               = 0x01U,
    LHA7668_FULL_POWER              = 0x02U,     // 高功率电源模式
} lha7668_power_mode_t;

typedef enum {
    LHA7668_MODE_CONT_READ          = 0x10U,  // 连续转换 + 连续读取模式
    LHA7668_MODE_CONT_CONV          = 0x0U,  // 默认连续转换
    LHA7668_MODE_SINGLE_SHOT        = 0x1U,  // 单次转换
    LHA7668_MODE_STANDY             = 0x2U,  // 待机
    LHA7668_MODE_SHUT_DOWN          = 0x3U,  // 关断
    LHA7668_MODE_IDLE               = 0x4U,  // 空闲
    LHA7668_MODE_INTL_OFFSET_CAL    = 0x5U,  // 内部失调校准
    LHA7668_MODE_INTL_GAIN_CAL      = 0x6U,  // 内部增益校准
    LHA7668_MODE_SYS_OFFSET_CAL     = 0x7U,  // 系统失调校准
    LHA7668_MODE_SYS_GAIN_CAL       = 0x8U,  // 系统增益校准
} lha7668_mode_t;

typedef enum {
	LHA7668_ERREN_MCLK_CNT			= 22U,
	LHA7668_ERREN_LDO_CAP_CHK_TEST	= 21U,
    LHA7668_ERREN_LDO_CAP_CHK_DIGITAL= 20U,
	LHA7668_ERREN_LDO_CAP_CHK_ANALOG= 19U,
	LHA7668_ERREN_ADC_CAL_ERR		= 18U,
	LHA7668_ERREN_ADC_CONV_ERR		= 17U,
	LHA7668_ERREN_ADC_SAT_ERR		= 16U,
	LHA7668_ERREN_AINP_OV_ERR		= 15U,
	LHA7668_ERREN_AINP_UV_ERR		= 14U,
	LHA7668_ERREN_AINM_OV_ERR		= 13U,
	LHA7668_ERREN_AINM_UV_ERR		= 12U,
	LHA7668_ERREN_REF_DET_ERR		= 11U,
	LHA7668_ERREN_DLDO_PSM_TRIP_TEST= 10U,
	LHA7668_ERREN_DLDO_PSM_ERR		= 9U,
	LHA7668_ERREN_ALDO_PSM_TRIP_TEST= 8U,
	LHA7668_ERREN_ALDO_PSM_ERR		= 7U,
	LHA7668_ERREN_SPI_IGNORE_ERR	= 6U,
	LHA7668_ERREN_SPI_SCLK_CNT_ERR	= 5U,
	LHA7668_ERREN_SPI_READ_ERR		= 4U,
	LHA7668_ERREN_SPI_WRITE_ERR		= 3U,
	LHA7668_ERREN_SPI_CRC_ERR		= 2U,
	LHA7668_ERREN_MM_CRC_ERR		= 1U,
	LHA7668_ERREN_ROM_CRC_ERR		= 0U,
} lha7668_error_t;

typedef enum {
    LHA7668_CLK_INTL                = 0x00U,     // 内部时钟
    LHA7668_CLK_INTL_AVL_OUTPUT     = 0x01U,
    LHA7668_CLK_EXT                 = 0x02U,
    LHA7668_CLK_EXT_DIV_4           = 0x03U,
} lha7668_clk_t;

typedef enum {
    LHA7668_SETUP_0                 = 0U,
    LHA7668_SETUP_1                 = 1U,
    LHA7668_SETUP_2                 = 2U,
    LHA7668_SETUP_3                 = 3U,
    LHA7668_SETUP_4                 = 4U,
    LHA7668_SETUP_5                 = 5U,
    LHA7668_SETUP_6                 = 6U,
    LHA7668_SETUP_7                 = 7U,
    LHA7668_SETUP_8                 = 8U,
} lha7668_setup_number_t;

typedef enum {
    LHA7668_VBIAS_0                 = 0U,
    LHA7668_VBIAS_1,
    LHA7668_VBIAS_2,
    LHA7668_VBIAS_3,
    LHA7668_VBIAS_4,
    LHA7668_VBIAS_5,
    LHA7668_VBIAS_6,
    LHA7668_VBIAS_7,
    LHA7668_VBIAS_8,
    LHA7668_VBIAS_9,
    LHA7668_VBIAS_10,
    LHA7668_VBIAS_11,
    LHA7668_VBIAS_12,
    LHA7668_VBIAS_13,
    LHA7668_VBIAS_14,
    LHA7668_VBIAS_15,
} lha7668_vbias_t;

typedef enum {
    LHA7668_IOUT_0                 = 0U,
    LHA7668_IOUT_1,
    LHA7668_IOUT_2,
    LHA7668_IOUT_3,
    LHA7668_IOUT_4,
    LHA7668_IOUT_5,
    LHA7668_IOUT_6,
    LHA7668_IOUT_7,
    LHA7668_IOUT_8,
    LHA7668_IOUT_9,
    LHA7668_IOUT_10,
    LHA7668_IOUT_11,
    LHA7668_IOUT_12,
    LHA7668_IOUT_13,
    LHA7668_IOUT_14,
    LHA7668_IOUT_15,
} lha7668_iout_t;

typedef enum {
    LHA7668_CURRENT_OFF             = 0U,
    LHA7668_CURRENT_50_UA,
    LHA7668_CURRENT_100_UA,
    LHA7668_CURRENT_250_UA,
    LHA7668_CURRENT_500_UA,
    LHA7668_CURRENT_750_UA,
    LHA7668_CURRENT_1000_UA,
} lha7668_current_t;

typedef enum {
	LHA7668_AIN0                    = 0x0U,      // 模拟输入通道
	LHA7668_AIN1,
	LHA7668_AIN2,
	LHA7668_AIN3,
	LHA7668_AIN4,
	LHA7668_AIN5,
	LHA7668_AIN6,
	LHA7668_AIN7,
	LHA7668_AIN8,
	LHA7668_AIN9,
	LHA7668_AIN10,
	LHA7668_AIN11,
	LHA7668_AIN12,
	LHA7668_AIN13,
	LHA7668_AIN14,
	LHA7668_AIN15,
	LHA7668_TEMP,
	LHA7668_AVSS,
	LHA7668_REF,
	LHA7668_DGND,
	LHA7668_AVDD_AVSS_6_P,
	LHA7668_AVDD_AVSS_6_N,
	LHA7668_IOVDD_DGND_6_P,
	LHA7668_IOVDD_DGND_6_N,
	LHA7668_ALDO_AVSS_6_P,
	LHA7668_ALDO_AVSS_6_N,
	LHA7668_DLDO_AVSS_6_P,
	LHA7668_DLDO_AVSS_6_N,
	LHA7668_V_20MV_P,
	LHA7668_V_20MV_M,
} lha7668_ain_t;

typedef enum {
    LHA7668_UNIPOLAR                    = 0U,
    LHA7668_BIPOLAR                     = 1U,        // 双极性模式
} lha7668_bipolar_t;

typedef enum {
    LHA7668_REF_REFIN1                  = 0x00U,
    LHA7668_REF_REFIN2                  = 0x01U,
    LHA7668_REF_INTERNAL                = 0x02U,     // 选择内部基准源并使能
    LHA7668_REF_AVDD                    = 0x03U,
    LHA7668_REF_REFOUT_IN               = 0x12U,    // 内部基准从REFOUT引脚提供      
} lha7668_ref_t;

typedef enum {
    LHA7668_PGA_X1                      = 0x00U,     // 增益 x 1
    LHA7668_PGA_X2                      = 0x01U,
    LHA7668_PGA_X4                      = 0x02U,
    LHA7668_PGA_X8                      = 0x03U,
    LHA7668_PGA_X16                     = 0x04U,
    LHA7668_PGA_X32                     = 0x05U,
    LHA7668_PGA_X64                     = 0x06U,
    LHA7668_PGA_X128                    = 0x07U,
} lha7668_pga_t;

typedef enum {
    LHA7668_BURNOUT_OFF                 = 0x00U,
    LHA7668_BURNOUT_0_5_UA              = 0x01U,    // 0.5uA
    LHA7668_BURNOUT_2_UA                = 0x02U,    // 2uA
    LHA7668_BURNOUT_4_UA                = 0x03U,    // 4uA
} lha7668_burnout_t;

typedef enum {
    LHA7668_FILTER_SINC4                = 0x00U,    // Sinc4数字滤波器
    LHA7668_FILTER_SINC3                = 0x40U,
    LHA7668_FILTER_SINC4_PLUS_SINC1     = 0x80U,    // Sinc4数字滤波器快速建立
    LHA7668_FILTER_SINC3_PLUS_SINC1     = 0xA0U,
    LHA7668_FILTER_POST_27_SPS          = 0xE4U,    // 后置滤波器27 SPS
    LHA7668_FILTER_POST_25_SPS          = 0xE6U,
    LHA7668_FILTER_POST_20_SPS          = 0xEAU,
    LHA7668_FILTER_POST_17_SPS          = 0xECU,   
    LHA7668_FILTER_SINC4_SINGLE_CYCLE               = 0x01U,    // Sinc4数字滤波器零延迟，多通道/单次转换/快速建立时无效
    LHA7668_FILTER_SINC4_WITH_REJ60                 = 0x10U,    // Sinc4数字滤波器50/60Hz同步抑制
    LHA7668_FILTER_SINC4_SINGLE_CYCLE_WITH_REJ60    = 0x11U,    // Sinc4数字滤波器零延迟带同步抑制
    LHA7668_FILTER_SINC3_SINGLE_CYCLE               = 0x41U,    
    LHA7668_FILTER_SINC3_WITH_REJ60                 = 0x50U,    
    LHA7668_FILTER_SINC3_SINGLE_CYCLE_WITH_REJ60    = 0x51U,     
} lha7668_filter_t;

typedef enum {
    LHA7668_POST_OFF                    = 0x00U,
    LHA7668_POST_27_SPS                 = 0x02U,
    LHA7668_POST_25_SPS                 = 0x03U,
    LHA7668_POST_20_SPS                 = 0x05U,
    LHA7668_POST_17_SPS                 = 0x06U,
} lha7668_post_filter_t;

typedef enum {
    LHA7668_STATUS_RDY_FLAG             = LHA7668_STATUS_RDY_MSK,
    LHA7668_STATUS_ERROR_FLAG           = LHA7668_STATUS_ERROR_FLAG_MSK,
    LHA7668_STATUS_POR_FLAG             = LHA7668_STATUS_POR_FLAG_MSK,
} lha7668_status_flag_t;

/* 2.2 LHA7668 寄存器结构体定义 ----------------------------------------------*/
/** 因为有跨字节的寄存器定义，无法使用位域和union的方式操作超过1字节的寄存器 **/
typedef union {
    struct {
        uint8_t RDY                 : 1;
        uint8_t ERROR_FLAG          : 1;  
        uint8_t                     : 1;
        uint8_t POR_FLAG            : 1;
        uint8_t CH_ACTIVE           : 4;  
    }BIT;
    uint8_t REG;
}lha7668_status_t;

typedef struct {
    uint32_t REG;
    lha7668_functional_state DOUT_RDY_DEL;
    lha7668_functional_state CONT_READ;
    lha7668_functional_state DATA_STATUS;   
    lha7668_functional_state CS_EN;
    lha7668_functional_state REF_EN; 
    lha7668_power_mode_t POWER_MODE;
    lha7668_mode_t MODE;
    lha7668_clk_t CLK_SEL;
} lha7668_adc_ctrl_t;

typedef struct {
    uint32_t REG;
	lha7668_functional_state ENABLE;
    lha7668_setup_number_t SETUP;       // Setup Number
    lha7668_ain_t AINP;
	lha7668_ain_t AINM;  
} lha7668_channel_t;

typedef struct {
    uint32_t REG[4];                    //0->Config; 1->FILTER; 2->OFFSET; 3->GAIN
	lha7668_bipolar_t BIPOLAR;          //Configuration Register Bits
	lha7668_functional_state REF_BUFP;
    lha7668_functional_state REF_BUFM;
	lha7668_functional_state AIN_BUFP;
    lha7668_functional_state AIN_BUFM;
	lha7668_ref_t REF_SEL;           
	lha7668_pga_t PGA;    
    lha7668_filter_t FILTER;            //Filter Register Bits
    //lha7668_functional_state REJ60;
    //lha7668_post_filter_t POST_FILTER;
    //lha7668_functional_state SINGLE_CYCLE;
    uint16_t FS;
    uint16_t BURNOUT;
} lha7668_setup_t;

/******************************************************************************
 * 3. LHA7668 驱动函数实现
 ******************************************************************************/
/* 3.1 LHA7668 驱动函数结构体定义 --------------------------------------------*/
typedef void (*lha7668_io_ptr)(void *);
typedef int32_t (*lha7668_read_write_ptr)(uint8_t *, uint8_t *, const uint16_t);
typedef void (*lha7668_set_ptr)(const void *, const uint32_t );
typedef void (*lha7668_reset_ptr)(const void *, const uint32_t );
typedef void (*lha7668_delay_us_ptr)(uint32_t microsecond);

typedef struct {
	void					*port;		            // 定义GPIO Port端口
	uint32_t				pin;		            // 定义GPIO Pin管脚   
} lhl_lha7668_io_alloc;

typedef struct {
	lhl_lha7668_io_alloc 	cs;                     // cs必须被分配
	lhl_lha7668_io_alloc	sync;                   // sync可选分配
}lhl_lha7668_io_t;

typedef union __LHA7668_ALIGN_4 {
    struct {
        uint8_t address;
        uint8_t content[0x0F];
    } parts;
    uint8_t data[0x10];                             // 发送数据
} lhl_lha7668_tx_t;

typedef union __LHA7668_ALIGN_4 {
    uint32_t content[0x04];
    uint8_t data[0x10];                             // 接收数据
} lhl_lha7668_rx_t;

typedef struct {                                    // 驱动函数主句柄
  	/** Component mandatory fields **/
  	lha7668_read_write_ptr      read_write;
    /** Component optional fields **/
  	lha7668_set_ptr             set;
  	lha7668_reset_ptr		    reset;
	lha7668_io_ptr			    io_alloc;
  	lha7668_delay_us_ptr	    delay_us;
  	/** Communication Data and Control **/
	lhl_lha7668_tx_t 	        tx;
    lhl_lha7668_rx_t 		    rx;
    lhl_lha7668_io_t	        io;
    uint16_t 			        size;
    /** ADC Data Result  **/
    uint8_t                     status;
    uint8_t                     crc;  
    uint32_t                    data;    
    /** ADC Register Map **/
    lha7668_channel_t           CHANNEL;
    lha7668_setup_t             SETUP;
    lha7668_adc_ctrl_t          ADC_CTRL;
    uint32_t                    ERREN;
} lhl_lha7668_ctx_t;

float rtd_resistance_to_temperature(float resistance);
float calculate_k_type_temperature(float emf_mv, float cold_junction_temp);
uint8_t calculate_crc8(const uint8_t *data, uint8_t length);

// 应用层函数
void LHL_LHA7668_Init(lhl_lha7668_ctx_t *ctx);
void LHL_LHA7668_Reset(lhl_lha7668_ctx_t *ctx);
void LHL_LHA7668_Start(lhl_lha7668_ctx_t *ctx, const lha7668_mode_t mode);
void LHL_LHA7668_Stop(lhl_lha7668_ctx_t *ctx);
void LHL_LHA7668_Resume(lhl_lha7668_ctx_t *ctx);

void LHL_LHA7668_Set_Channel(lhl_lha7668_ctx_t *ctx, uint8_t channel);
void LHL_LHA7668_Set_Setup(lhl_lha7668_ctx_t *ctx, lha7668_setup_number_t setup);
void LHL_LHA7668_Set_ADC(lhl_lha7668_ctx_t *ctx);

void LHL_LHA7668_Set_Diagnostics(lhl_lha7668_ctx_t *ctx, lha7668_error_t error, lha7668_functional_state new_state);
void LHL_LHA7668_Set_Mode(lhl_lha7668_ctx_t *ctx, lha7668_mode_t mode);
void LHL_LHA7668_Set_Power(lhl_lha7668_ctx_t *ctx, lha7668_power_mode_t power);
void LHL_LHA7668_Set_Reference(lhl_lha7668_ctx_t *ctx, lha7668_functional_state new_state);
void LHL_LHA7668_Set_Burnout(lhl_lha7668_ctx_t *ctx, lha7668_setup_number_t setup, lha7668_burnout_t magnitude);
void LHL_LHA7668_Set_VBias(lhl_lha7668_ctx_t *ctx, lha7668_vbias_t vbias, lha7668_functional_state new_state);
void LHL_LHA7668_Set_IOut(lhl_lha7668_ctx_t *ctx, lha7668_iout_t iout0_channel, lha7668_iout_t iout1_channel, lha7668_current_t current);
void LHL_LHA7668_Set_IOut0(lhl_lha7668_ctx_t *ctx, lha7668_iout_t iout, lha7668_current_t current);
void LHL_LHA7668_Set_IOut1(lhl_lha7668_ctx_t *ctx, lha7668_iout_t iout, lha7668_current_t current);

uint8_t LHL_LHA7668_Get_ID(lhl_lha7668_ctx_t *ctx);
void LHL_LHA7668_Get_Data(lhl_lha7668_ctx_t *ctx);
float LHL_LHA7668_Get_mVoltage(const uint32_t raw, const lha7668_bipolar_t bipolar, const lha7668_pga_t gain, const float ref_mV);
float LHL_LHA7668_Get_Ohm(const uint32_t raw, const lha7668_bipolar_t bipolar, const lha7668_pga_t gain, const float ref_Ohm);
float LHL_LHA7668_Get_Temperature(const uint32_t raw);
lha7668_flag_status LHL_LHA7668_Get_Flag(lhl_lha7668_ctx_t *ctx, const lha7668_status_flag_t flag);

void LHL_LHA7668_Dump_Register(lhl_lha7668_ctx_t *ctx, const uint8_t start_address, const uint8_t end_address, uint32_t *buf);
void LHL_LHA7668_Dump_Register_CRC(lhl_lha7668_ctx_t *ctx, const uint8_t start_address, const uint8_t end_address, uint32_t *buf, uint8_t *crc);

// 协议层函数
uint32_t LHL_LHA7668_Read_Register(lhl_lha7668_ctx_t *ctx, const uint8_t address, const uint8_t size);
void LHL_LHA7668_Write_Register(lhl_lha7668_ctx_t *ctx, const uint8_t address, const uint32_t content, const uint8_t size);

// 时序层函数
void __LHL_LHA7668_Read_Data(lhl_lha7668_ctx_t *ctx);
void __LHL_LHA7668_Read_Write_Raw(lhl_lha7668_ctx_t *ctx);
uint32_t __LHL_Array_to_Int(const uint8_t *data);

// 接口层函数
int32_t LHA7668_Platform_ReadWrite(uint8_t *txdata, uint8_t *rxdata, const uint16_t size);
void LHA7668_Platform_Init(void *handle);
void LHA7668_Platform_Set(const void *port, const uint32_t pin);
void LHA7668_Platform_Reset(const void *port, const uint32_t pin);
void LHA7668_Platform_Delay_uS(volatile uint32_t cycles);

#ifdef __cplusplus
}
#endif
#endif

/*********************************End of File**********************************/
