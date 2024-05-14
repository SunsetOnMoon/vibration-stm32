/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "vibric.h"
#include "ds1307.h"
#include "command_viric.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_BUFFER_SIZE 1024//512//256
#define BUFFER_SIZE ADC_BUFFER_SIZE// * 2
#define VIBRIC_DATA_SIZE 8192//65536//8192
#define COMMAND_SIZE 21
#define SYS_FREQ 32000000
#define TIM4_PRESCALER 80
#define FREQ_TEST 1 // 0 - NO TEST; 1 - WRITE IN FILE REGISTER TIME AND NUMBER OF ELEMENTS FOR FREQUENCY TEST
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint16_t adcBuffer[ADC_BUFFER_SIZE] = {0, };
FATFS fs = {0}; // ПРАВКА!
FIL fil;
FRESULT fresult;
UINT br, bw;
char buffer[BUFFER_SIZE] = {0, };
vibric_header* vibric;
UINT size_control = 0;
char is_started, is_stoped;
int delay_time;
uint8_t receive_uart_buffer[25];

#if FREQ_TEST == 1
	int start_time;
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM4_Init(void);
static void MX_SPI2_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int getbufferLength(char buffer[]) {
	int i = 0;
	while (buffer[i] != '\0') {
		i++;
	}
	return i;
}

void startWritingInFile() {
	MX_FATFS_Init();
	fresult = f_mount(&fs, "", 1);
	while (fresult != FR_OK) {
		HAL_Delay(50);
		MX_FATFS_DeInit();
		HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
		HAL_Delay(50);
		MX_FATFS_Init();
		fresult = f_mount(&fs, "", 1);
	}
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
	char* file_datetime = DS1307_GetCurrentDateTimeForFile(); // datetime for filename
	if (file_datetime == NULL) {
		fresult = f_open(&fil, "data.dat", FA_CREATE_ALWAYS | FA_WRITE);
	} else {
		char* filename = (char*)malloc(8 * sizeof(char));
		sprintf(filename, "%s.dat", file_datetime);
//		filename[16] = '\0';
		fresult = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
		char* datetime_label = DS1307_GetCurrentDateTime(); // datetime для метки внутри файла
		if (datetime_label != NULL) {
			char* label = (char*)malloc(13 * sizeof(char));
			sprintf(label, "%s\n", datetime_label);
			fresult = f_write(&fil, label, 13, &bw);
			free(label);
		} else {
			fresult = f_write(&fil, "UNDEFINED\n", 10, &bw);
		}
		free(datetime_label);
		free(filename);
	}
	free(file_datetime);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adcBuffer, ADC_BUFFER_SIZE);
	HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_4);
#if FREQ_TEST == 1
	start_time = HAL_GetTick();
#endif
}

void stopWritingInFile() {
#if FREQ_TEST == 1
	int writing_time = HAL_GetTick() - start_time;
	char info[100];
	sprintf(info, "\nWriting Time: %d\nCount of elements: %d\n", writing_time, size_control);
	fresult = f_write(&fil, info, getbufferLength(info), &bw);
#endif

	is_started = 0;
	size_control = 0;
	HAL_ADC_Stop_DMA(&hadc1);
	HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_4);
	fresult = f_close(&fil);
	fresult = f_mount(0, "", 0);
	MX_FATFS_DeInit();
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
}

void writeDataFromADCToFileByTime(int offset, int adc_buffer_size)
{
	if (size_control != VIBRIC_DATA_SIZE) {
		memset(buffer, 0, BUFFER_SIZE);
		memcpy(buffer, adcBuffer + offset, BUFFER_SIZE);
		fresult = f_write(&fil, buffer, BUFFER_SIZE, &bw);
		size_control += adc_buffer_size;
	}
	else {
		stopWritingInFile();
	}

}

void writeDataFromADCToFile(int offset, int adc_buffer_size)
{
	if ((HAL_GetTick() - start_time) <= 3000) {
	memset(buffer, 0, BUFFER_SIZE);
	memcpy(buffer, adcBuffer + offset, BUFFER_SIZE);
	fresult = f_write(&fil, buffer, BUFFER_SIZE, &bw);
	size_control += adc_buffer_size; // optional
	} else {
		stopWritingInFile();
	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
	if ((hadc->Instance == ADC1) && (is_started)) {
		writeDataFromADCToFile(0, ADC_BUFFER_SIZE / 2);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if ((hadc->Instance == ADC1) && (is_started))
	{
		writeDataFromADCToFile(ADC_BUFFER_SIZE / 2, ADC_BUFFER_SIZE / 2);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == BTN_STOP_Pin) {
		if (HAL_GPIO_ReadPin(BTN_STOP_GPIO_Port, BTN_STOP_Pin) == GPIO_PIN_RESET)
			delay_time = HAL_GetTick();
		else  if (HAL_GPIO_ReadPin(BTN_START_GPIO_Port, BTN_STOP_Pin) == GPIO_PIN_SET){
			if (((HAL_GetTick() - delay_time) >= 20) && ((HAL_GetTick() - delay_time) <= 1000) && is_started) {
				stopWritingInFile();
			}
			else if ((HAL_GetTick() - delay_time) > 1000) {
				HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
				NVIC_SystemReset();
			}
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (is_started) {
		HAL_UART_Transmit_IT(&huart2, (uint8_t*)"(DLIS,0,000000000000)", COMMAND_SIZE);
	}
	else {
		Vibric_CommandTypeDef command;
		command = Vibric_ParseCommand((char*)receive_uart_buffer);
		if (strcmp(command.name, "SAMP") == 0) {
			switch (command.code) {
			case 0: {
				break;
			}
			case 1: {
				int freq = Vibric_ConvertCommandDataToInt(command.data);
				int counter = (SYS_FREQ / (freq * TIM4_PRESCALER)) - 1;
//				TIM4->CCR4 = counter;
				TIM4->ARR = counter;
				HAL_UART_Transmit_IT(&huart2, (uint8_t*)"(SAMP,1,000000000000)", 21);
			}
			}
		}
		else if (strcmp(command.name, "TIME") == 0) {
			switch (command.code) {
			case 0:
				char* time = DS1307_GetCurrentDateTimeCommand();
				if (time == NULL) {
					HAL_UART_Transmit_IT(&huart2, (uint8_t*)"(TIME,2,111111111111)", 21);
				}
				else
					HAL_UART_Transmit_IT(&huart2, (uint8_t*)time, 21);
				free(time);
				break;
			case 1: {
				DS1307_SetDateTime(command.data);
				HAL_UART_Transmit_IT(&huart2, (uint8_t*)"(TIME,3,000000000000)", 21);
				break;
			}
			case 2: {
				// TODO Get launch time
				break;
			}
			case 3: {
				// TODO Set launch time
				break;
			}
			case 4: {
				// TODO Get end time
				break;
			}
			case 5: {
				// TODO Set end time
				break;
			}
			}
		}
	}
	memset(receive_uart_buffer, 0, sizeof(receive_uart_buffer));
	HAL_UART_Receive_IT(&huart2, receive_uart_buffer, 21);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_FATFS_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  MX_SPI2_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  is_started = 0;
  is_stoped = 0;

  // RTC Initialization
  DS1307_Init(&hi2c2);
  DS1307_SetRate(DS1307_32768Hz);

  // UART Rx Initialization
  memset(receive_uart_buffer, 0, 25);
  HAL_UART_Receive_IT(&huart2, receive_uart_buffer, 21);

  // Test init of FatFS
  MX_FATFS_Init();
  fresult = f_mount(&fs, "", 1);
  while (fresult != FR_OK) {
	  HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	  HAL_Delay(500);
	  HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
  }
  fresult = f_mount(0, "", 0);
  MX_FATFS_DeInit();

  HAL_ADCEx_Calibration_Start(&hadc1);
  for (int i = 0; i < 10; i++) {
	  HAL_Delay(100);
	  HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
  	  HAL_Delay(100);
  	  HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if ((HAL_GPIO_ReadPin(BTN_START_GPIO_Port, BTN_START_Pin) == GPIO_PIN_RESET) && !is_started) {
		  is_started = 1;
		  startWritingInFile();
	  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T4_CC4;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 79;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 399;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SD_CS_Pin|LED_GREEN_Pin|LED_RED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BTN_STOP_Pin */
  GPIO_InitStruct.Pin = BTN_STOP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_STOP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_START_Pin */
  GPIO_InitStruct.Pin = BTN_START_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_START_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SD_CS_Pin LED_GREEN_Pin LED_RED_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin|LED_GREEN_Pin|LED_RED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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

#ifdef  USE_FULL_ASSERT
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
