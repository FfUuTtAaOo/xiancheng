/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "calibration.h"
#include "filter.h"
#include "lha7668-8.h"
#include "self_test.h"
#include "rs485_cmd.h"
#include "sensor_types.h"
#include "flash_storage.h"
#include "at24c04.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t g_tick_ms = 0;

/* Shared global state (declared in sensor_types.h) */
volatile sensor_data_t   g_sensor;
volatile system_state_t  g_sys;
config_t                 g_config = {
    .sn         = "HS-01234567",
    .fw_version = 0x00010702,
};
decouple_matrix_t        g_matrix;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define ADC_DATA_BUFF_LEN       64
#define ADC_CHANNEL_NUM         6
#define KILOGRAM_TO_NEWTON      9.8

static uint8_t adc_data_int_count = 0;
static uint8_t adc_data_usr_count = 0;
static uint32_t adc_data_buff[ADC_DATA_BUFF_LEN][6] = { 0 };


static float adc_data_calculate(uint8_t index)
{
    return (float)((int32_t)adc_data_buff[adc_data_usr_count][index] - 0x800000) * (1.0f / 8388608.0f) * 2500;
}

static void adc_data_proc(float data[6])
{
    int i = 0;
    if (g_sys.data_format == 0) {
        for (i = 0; i < ADC_CHANNEL_NUM; i++) {
            g_sensor.force[i] = data[i];
        }
    } else if (g_sys.data_format == 1) {
        g_sensor.force[0] = g_matrix.m[0][0] * data[0] / KILOGRAM_TO_NEWTON + g_matrix.m[0][1] * data[1] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[0][2] * data[2] / KILOGRAM_TO_NEWTON + g_matrix.m[0][3] * data[3] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[0][4] * data[4] / KILOGRAM_TO_NEWTON + g_matrix.m[0][5] * data[5] / KILOGRAM_TO_NEWTON;

        g_sensor.force[1] = g_matrix.m[1][0] * data[0] / KILOGRAM_TO_NEWTON + g_matrix.m[1][1] * data[1] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[1][2] * data[2] / KILOGRAM_TO_NEWTON + g_matrix.m[1][3] * data[3] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[1][4] * data[4] / KILOGRAM_TO_NEWTON + g_matrix.m[1][5] * data[5] / KILOGRAM_TO_NEWTON;

        g_sensor.force[2] = g_matrix.m[2][0] * data[0] / KILOGRAM_TO_NEWTON + g_matrix.m[2][1] * data[1] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[2][2] * data[2] / KILOGRAM_TO_NEWTON + g_matrix.m[2][3] * data[3] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[2][4] * data[4] / KILOGRAM_TO_NEWTON + g_matrix.m[2][5] * data[5] / KILOGRAM_TO_NEWTON;

        g_sensor.force[3] = g_matrix.m[3][0] * data[0] / KILOGRAM_TO_NEWTON + g_matrix.m[3][1] * data[1] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[3][2] * data[2] / KILOGRAM_TO_NEWTON + g_matrix.m[3][3] * data[3] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[3][4] * data[4] / KILOGRAM_TO_NEWTON + g_matrix.m[3][5] * data[5] / KILOGRAM_TO_NEWTON;

        g_sensor.force[4] = g_matrix.m[4][0] * data[0] / KILOGRAM_TO_NEWTON + g_matrix.m[4][1] * data[1] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[4][2] * data[2] / KILOGRAM_TO_NEWTON + g_matrix.m[4][3] * data[3] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[4][4] * data[4] / KILOGRAM_TO_NEWTON + g_matrix.m[4][5] * data[5] / KILOGRAM_TO_NEWTON;

        g_sensor.force[5] = g_matrix.m[5][0] * data[0] / KILOGRAM_TO_NEWTON + g_matrix.m[5][1] * data[1] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[5][2] * data[2] / KILOGRAM_TO_NEWTON + g_matrix.m[5][3] * data[3] / KILOGRAM_TO_NEWTON +
                    g_matrix.m[5][4] * data[4] / KILOGRAM_TO_NEWTON + g_matrix.m[5][5] * data[5] / KILOGRAM_TO_NEWTON;
    } else if (g_sys.data_format == 2) {
        g_sensor.force[0] = g_matrix.m[0][0] * data[0] + g_matrix.m[0][1] * data[1] +
                    g_matrix.m[0][2] * data[2] + g_matrix.m[0][3] * data[3] +
                    g_matrix.m[0][4] * data[4] + g_matrix.m[0][5] * data[5];

        g_sensor.force[1] = g_matrix.m[1][0] * data[0] + g_matrix.m[1][1] * data[1] +
                    g_matrix.m[1][2] * data[2] + g_matrix.m[1][3] * data[3] +
                    g_matrix.m[1][4] * data[4] + g_matrix.m[1][5] * data[5];

        g_sensor.force[2] = g_matrix.m[2][0] * data[0] + g_matrix.m[2][1] * data[1] +
                    g_matrix.m[2][2] * data[2] + g_matrix.m[2][3] * data[3] +
                    g_matrix.m[2][4] * data[4] + g_matrix.m[2][5] * data[5];

        g_sensor.force[3] = g_matrix.m[3][0] * data[0] + g_matrix.m[3][1] * data[1] +
                    g_matrix.m[3][2] * data[2] + g_matrix.m[3][3] * data[3] +
                    g_matrix.m[3][4] * data[4] + g_matrix.m[3][5] * data[5];

        g_sensor.force[4] = g_matrix.m[4][0] * data[0] + g_matrix.m[4][1] * data[1] +
                    g_matrix.m[4][2] * data[2] + g_matrix.m[4][3] * data[3] +
                    g_matrix.m[4][4] * data[4] + g_matrix.m[4][5] * data[5];

        g_sensor.force[5] = g_matrix.m[5][0] * data[0] + g_matrix.m[5][1] * data[1] +
                    g_matrix.m[5][2] * data[2] + g_matrix.m[5][3] * data[3] +
                    g_matrix.m[5][4] * data[4] + g_matrix.m[5][5] * data[5];
    }

    for (i = 0; i < ADC_CHANNEL_NUM; i++) {
        g_sensor.force[i] = FloatFilter_UpdateChannel(i, g_sensor.force[i]);
    }
}

void uart_debug(const char *msg)
{
    HAL_UART_Transmit(&huart1, (const uint8_t *)msg, (uint16_t)strlen(msg), 100);
}

/* Interrupt function callback -----------------------------------------------*/
void USART1_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
        if (huart1.hdmarx) {
            uint16_t rem = (uint16_t)__HAL_DMA_GET_COUNTER(huart1.hdmarx);
            rs485_rx_idle_cb(rem);
        }
    }
    HAL_UART_IRQHandler(&huart1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        g_tick_ms++;

        if (g_tick_ms % 3 == 0) {
            uint32_t raw[ADC_CHANNEL_NUM];
            uint8_t  i;
            /* 单颗 LHA7668 六通道 = FX/FY/FZ/TX/TY/TZ。
               整组读取，任一通道超时/序列异常则整组不更新，
               数组各通道保留自己上一次的有效值，绝不相互错位 */
            if (int_adcData_6ch(raw) == 0) {
                for (i = 0; i < ADC_CHANNEL_NUM; i++) {
                    adc_data_buff[adc_data_int_count][i] = raw[i];
                }
            }
            adc_data_int_count++;
            if (adc_data_int_count >= 64) {
                adc_data_int_count = 0;
            }
        }
    }
}

/* ---- Continuous RS485 data send state ---- */
static uint32_t g_last_rs485_send_ms;

/* ---- PA12 streaming heartbeat ---- */
#define LED_BLINK_HALF_PERIOD_MS   500U    /* 500 ms on / 500 ms off = 1 Hz */
static uint32_t g_led_toggle_ms;

void send_string(uint8_t *str, uint16_t len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, len, 500);
}

uint8_t mb_buf_send[33] = { 0 };

static void rs485_send_continuous(void)
{
    uint32_t interval = (g_sys.freq_mode == 0) ? 2U : 1U;  /* 500Hz or 1kHz */
    if ((g_tick_ms - g_last_rs485_send_ms) >= interval) {
        g_last_rs485_send_ms = g_tick_ms;

        /* Only reached while g_sys.send_mode == 1 (checked by the caller).
           The frame layout — AA 55 02 [6×float32] 0D 0A, the 00 00 80 7F
           debug frame, or nothing at all — is chosen by debug_mode. */
        rs485_send_data_frame(RS485_CMD_START);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//  uint8_t at24c04_write_buffer[32] = { 0 };

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
    /* Restore the persisted baud rate (written by RS485_CMD_SET_BAUD).
       Fall back to 460800 when the EEPROM is blank or holds an unsupported
       value, so a corrupt entry can never brick the communication link. */
    {
        uint8_t  baud_buf[4] = { 0 };
        uint32_t baud = 0;

        at24c04_read(UART_BAUD_ADDR, baud_buf, 4);
        baud = ((uint32_t)baud_buf[0])
             | ((uint32_t)baud_buf[1] << 8)
             | ((uint32_t)baud_buf[2] << 16)
             | ((uint32_t)baud_buf[3] << 24);

        if (baud == 460800U || baud == 256000U || baud == 115200U ||
            baud == 19200U  || baud == 9600U) {
            uart1_bsp = baud;
        } else {
            uart1_bsp = 460800;
        }
    }
    CUSTOM_USART1_Init();

    calib_init();
    uart_debug("Data Transformation Layer Init OK\r\n");
    FloatFilter_Init();
    uart_debug("FloatFilter Init OK\r\n");

    lha7668_init(ADC_SAMPLING_RATE);
    uart_debug("LHA7668 OK\r\n");

    uart_debug("--- RS485 self-test ---\r\n");
    rs485_init(&huart1, rs485_cmd_dispatch);
    uint32_t st_rs485 = self_test_rs485();
    uart_debug(st_rs485 == ST_ERR_NONE ? "RS485 OK (USART+DMA)\r\n" : "RS485 FAIL\r\n");

    flash_load_all();

    /* Restore the data output unit written by 0x33 (kg) / 0x35 (N).
       1 → kg, 2 → N, anything else → N. */
    {
        uint8_t fmt = 0xFF;

        at24c04_read(DATA_FORMAT_ADDR, &fmt, 1);
        g_sys.data_format = (fmt == 1 || fmt == 2) ? fmt : 2;
    }

    HAL_TIM_Base_Start_IT(&htim2);   /* Start 1kHz timer */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
        {
            float adc_tmp[ADC_CHANNEL_NUM] = { 0 };

            if (adc_data_usr_count != adc_data_int_count) {
                adc_data_usr_count = adc_data_int_count;
                for (uint8_t i = 0; i < ADC_CHANNEL_NUM; i++) {
                    adc_tmp[i] = adc_data_calculate(i);

                    if (g_sys.zero_calib_busy == 0) {
                        adc_tmp[i] -= g_sensor.force_zero[i];
                    }
                }

                if (g_sys.zero_calib_busy == 1) {
                    calib_zero_cale(adc_tmp);
                }

                adc_data_proc(adc_tmp);
            }
        }

        if (calib_zero_is_done()) {
            g_sys.zero_calib_busy = 0;
            calib_zero_over();
            flash_save_zero();
        }

        if (g_sys.send_mode == 1) {
            rs485_send_continuous();
        } else {
            g_last_rs485_send_ms = g_tick_ms;   /* reset timer on stop */
        }

        /* PA12 heartbeat: toggles every 500 ms (1 Hz blink) while frames are
           actually going out — i.e. after AA 55 02 0D 0A and while the output
           format is one that transmits. Held low as soon as output stops. */
        {
            uint8_t streaming = (g_sys.send_mode == 1) &&
                                (g_sys.debug_mode == 0 || g_sys.debug_mode == 1);

            if (streaming) {
                if ((uint32_t)(g_tick_ms - g_led_toggle_ms) >= LED_BLINK_HALF_PERIOD_MS) {
                    g_led_toggle_ms = g_tick_ms;
                    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_12);
                }
            } else {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
                g_led_toggle_ms = g_tick_ms;    /* keep the phase aligned */
            }
        }

        rs485_process();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
