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
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f7xx_hal.h"
#include "fatfs.h"
#include "bsp_driver_sd.h"
#include <string.h>
#include <stdio.h>
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

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SDMMC1_SD_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* FATFS Değişkenleri */

int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        HAL_UART_Transmit(&huart1, (uint8_t *)&ptr[i], 1, HAL_MAX_DELAY);
    }
    return len;
}

uint8_t workBuffer[_MAX_SS*8];
void sd_card_fromat() {
	memset(workBuffer,0,sizeof(workBuffer));
	printf("FatFs: Start formatting SD Card..\n");
	uint32_t start=HAL_GetTick();
	if (f_mkfs((TCHAR const*) SDPath, FM_ANY, 0, workBuffer, sizeof(workBuffer))
			!= FR_OK) {
		/* FatFs Format Error */
		printf("FatFs: Failed to format SD Card!!\n");
		Error_Handler();
	}
	printf("FatFs: formatted SD Card...%lu ms\n",HAL_GetTick()-start);
}

volatile uint8_t workBuffer2[16*4*512];	//512* ..
/*
 * 50mhz
 * 	 noname FatFs: Write speed: 77.00 KB/s	 512b	buffer
 * 	 		FatFs: Write speed: 1595.02 KB/s 32kb	buffer
* 	  UHS-1 FatFs: Write speed: 430.61 KB/s	 512b	buffer
* 			FatFs: Write speed: 951.26 KB/s	 1kb	buffer
 * 			FatFs: Write speed: 1882.82 KB/s 2kb	buffer
 *			FatFs: Write speed: 3494.82 KB/s 4kb	buffer
 *			FatFs: Write speed: 5688.95 KB/s 8kb	buffer
 *			FatFs: Write speed: 6243.90 KB/s 16kb	buffer
 *			FatFs: Write speed: 6249.35 KB/s 32kb	buffer
 *			FatFs: Failed to open/create file!! 64kb buffer error
 *
 *50 mhz 56kb buffer FatFs: Write speed: 9524.16 KB/s
 */
unsigned long mb=20;//test mb
void sd_card_write_test() {

	for (int i = 0; i < sizeof(workBuffer2); i++) {
		workBuffer2[i] = 'A';
	}

	printf("FatFs: Write %lu mb test started\n", mb);

	// Open or create a file on SD card (replace "test.txt" with your desired file name)
	if (f_open(&SDFile, "test.txt", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
		printf("FatFs: Failed to open/create file!!\n");
		return;
	}

	unsigned long totalBytesWritten = 0;
	unsigned long totalBytes = mb*1024 * 1024;

	uint32_t start=HAL_GetTick();
	while (totalBytesWritten < totalBytes) {
		unsigned long bytesToWrite = sizeof(workBuffer2);
		unsigned long bytesWrittenThisTime = 0;
	    FRESULT res = f_write(&SDFile, workBuffer2, bytesToWrite, &bytesWrittenThisTime);

	    if (res != FR_OK) {
	    	printf("FatFs: Write error, error code: %d\n", res);
	        f_close(&SDFile);
	        return;
	    }

	    if (bytesWrittenThisTime == 0) {
	    	printf("FatFs: Write failed, no bytes written!\n");
	        f_close(&SDFile);
	        return;
	    }

	    totalBytesWritten += bytesWrittenThisTime;
	}

	if (f_close(&SDFile) != FR_OK) {
		printf("FatFs: Failed to close file!!\n");
	}

	unsigned long elapsedTime = HAL_GetTick() - start;

	// Yazma hızını hesapla (Byte/saniye)
	float writeSpeed = (float)totalBytesWritten / (elapsedTime/1000.0f);  // MB/s cinsinden yazma hızı
	printf("FatFs: Write total time: %lu ms\n", elapsedTime);
	printf("FatFs: Write speed: %.2f KB/s\n", writeSpeed / 1024);  // Yazma hızını KB/s cinsinden yazdır

}
void sd_card_read_test(){

    printf("FatFs: Read %lu MB test started\n", mb);

    // Test dosyasını aç
    if (f_open(&SDFile, "test.txt", FA_READ) != FR_OK) {
        printf("FatFs: Failed to open file for reading!!\n");
        return;
    }
    volatile uint32_t totalBytes = mb*1024 * 1024;
    volatile uint32_t totalBytesRead = 0;
    uint32_t startRead = HAL_GetTick();

    while (totalBytesRead < totalBytes) {
        volatile UINT bytesReadThisTime = 0;
        FRESULT res = f_read(&SDFile, workBuffer2, sizeof(workBuffer2), &bytesReadThisTime);

        if (res != FR_OK) {
            printf("FatFs: Read error, error code: %d\n", res);
            f_close(&SDFile);
            return;
        }

        if (bytesReadThisTime == 0) {
            printf("FatFs: Read failed, no bytes read!\n");
            f_close(&SDFile);
            return;
        }

        totalBytesRead += bytesReadThisTime;
    }

    if (f_close(&SDFile) != FR_OK) {
        printf("FatFs: Failed to close file after reading!!\n");
    }

    unsigned long elapsedReadTime = HAL_GetTick() - startRead;
    float readSpeed = (float)totalBytesRead / (elapsedReadTime / 1000.0f);

    printf("FatFs: Read total time: %lu ms\n", elapsedReadTime);
    printf("FatFs: Read speed: %.2f KB/s\n", readSpeed / 1024);

}
void SD_Card_Test(void) {
	FATFS_UnLinkDriver(SDPath);
	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0) {
		if (f_mount(&SDFatFS, (TCHAR const*) SDPath, 0) != FR_OK) {
			printf("FatFs: Failed to mount SD Card!!\n");
			Error_Handler();
		}

		sd_card_fromat();
		if (f_opendir(&SDFile, (TCHAR const*) "0:/") == FR_OK) {
			__NOP();
		}
		sd_card_write_test();
		sd_card_read_test();

		FATFS_UnLinkDriver(SDPath);
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

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  setvbuf(stdout, NULL, _IONBF, 0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	SD_Card_Test();

	while (1) {

		HAL_Delay(5000);
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SDMMC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC1_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC1_Init 0 */

  /* USER CODE END SDMMC1_Init 0 */

  /* USER CODE BEGIN SDMMC1_Init 1 */

  /* USER CODE END SDMMC1_Init 1 */
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC1_Init 2 */
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;// Set to 1-bit mode initially

  // Once the board is initialized and the pins are correctly configured,
  // the BSP will automatically attempt to switch to 4-bit mode.
  //When 4-bit mode is active, it not working without DMA. I don't understand the reason.

  /* USER CODE END SDMMC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin : uSD_Detect_Pin */
  GPIO_InitStruct.Pin = uSD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(uSD_Detect_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
     ex: UART_Print("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
