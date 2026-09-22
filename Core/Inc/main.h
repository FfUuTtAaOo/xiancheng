/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void uart_debug(const char *msg);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define ADC_DRDY_Pin GPIO_PIN_0
#define ADC_DRDY_GPIO_Port GPIOB
#define ADC_CLKIN_Pin GPIO_PIN_8
#define ADC_CLKIN_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* EEPROM address holding the 4-byte (little-endian) UART baud rate */
#define UART_BAUD_ADDR        0x78

/* EEPROM address holding the 1-byte data output unit (1 = kg, 2 = N) */
#define DATA_FORMAT_ADDR      0x7C

#define UART_BUFFER_SIZE 64

#define U32_TO_U8(u32, u8, offset)  (u8) = (((u32) >> (offset)) & 0xFF)
#define UINT8_EQUAL(a, b) ((a) == (b))
#define U8_TO_U32_SAFE(u8_val, u32_val, index) \
  ({ \
    assert((index) < 4); \
    ((u32_val & ~(0xFFUL << ((index) * 8))) | (((uint32_t)(u8_val)) << ((index) * 8))); \
  })

#define TRUE 1
#define FALSE 0

typedef union {
    float f32;
    uint32_t u32;
} floatuint_t;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
