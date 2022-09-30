/*
This file contain 2 functions that were written with the help of the demo code provided by Waveshare,
and 2 functions written by STMicroelectronics

Most of the initialization for the fingerprint sensor can be found in util.c written by us. 
However, the USART1 initialization is found here (calling usart.c -> file written by STMicroelectronics) using the HAL libraries 
as the library functions provided by Waveshare rely on the Usart1_ReceiveStruct.RX_pData variable which is getting build with 
the HAL library

File: STMicroelectrionicsStartup.c
Purpose: Functions to initialize the USART1 and the clock

Author: Waveshare.com or STMicroelectronics (see comment above functions), modified by Erika McCluskey, Hayden Jin
*/

#include "STMicroelectrionicsStartup.h"
#include "usart.h"
#include "commands.h"
#include "fingerprint.h"

//this function was written with the help of the code provided by Waveshare for the fingerprint sensor
//the program can be found here: https://www.waveshare.com/wiki/UART_Fingerprint_Sensor_(C) under the Demo code section
void usart1_setup(void){
		HAL_Init();
		
		MX_USART1_UART_Init();
		
		GPIOB->BRR = GPIO_BRR_BR5; 
		HAL_Delay(300); 
		GPIOB->BSRR = GPIO_BSRR_BS5;
		HAL_Delay(300);  // Wait for module to start
		GPIOA->BRR = GPIO_BRR_BR5;
							 
		__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); 
		HAL_UART_Receive_DMA(&huart1, Usart1_ReceiveStruct.RX_pData, RX_LEN);   
}

//this function was written with the help of the code provided by Waveshare for the fingerprint sensor
//the program can be found here: https://www.waveshare.com/wiki/UART_Fingerprint_Sensor_(C) under the Demo code section
void device_startup(void){
		// while system is not ready, flash red light			
	// to make ready, reset the board w/ black user button
	while(SetcompareLevel(5) != 5)
	{
		HAL_Delay(1000); 
		turn_on_red_LED();
		delay(3000000);
		turn_off_red_LED();
		delay(3000000);
	}
}


/**
  * @brief System Clock Configuration
  * @retval None

This function was written by STMicroelectronics, copyright notice can be found in file usart.c
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
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
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None

This function was written by STMicroelectronics, copyright notice can be found in file usart.c
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
