/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dht11.h"
#include "HDG12864F1.h"
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
TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */
volatile uint8_t ui_mode = 0;
volatile uint8_t ui_dirty = 0;
uint8_t  temperature=0;
uint8_t  humidity=0;
uint8_t temup = 75;
uint8_t temlimit = 25;
uint8_t humup = 70;
uint8_t humlimit = 20;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  dht11_init(); 
  Delay_ms(10);
  dht11_read_data(&temperature, &humidity);            

  Lcd_GPIO_Configuration();                    
  HDG12864F1_Init();                       
  HDG12864F1_Clear();                      
  HAL_Delay_us(10);
  DrawStatic();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if (ui_dirty) {
		ui_dirty = 0;
		HDG12864F1_Clear();
		DrawStatic();
	}
	MainUi();
	HAL_Delay_us(200);
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

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_1|GPIO_PIN_4
                          |GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pins : MinusButton_Pin AddButton_Pin ChangeButton_Pin */
  GPIO_InitStruct.Pin = MinusButton_Pin|AddButton_Pin|ChangeButton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA5 PA6 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void DrawStatic() {
  HDG12864F1_WriteChineseCharUpper(cCharWenUpper, CHINESE_COLUMN_0, 0);
  HDG12864F1_WriteChineseCharLower(cCharWenLower, CHINESE_COLUMN_0, 0);

  HDG12864F1_WriteChineseCharUpper(cCharDuUpper, CHINESE_COLUMN_1, 0);
  HDG12864F1_WriteChineseCharLower(cCharDuLower, CHINESE_COLUMN_1, 0);

  

  HDG12864F1_WriteChineseCharUpper(cCharShiUpper, CHINESE_COLUMN_0, 2);
  HDG12864F1_WriteChineseCharLower(cCharShiLower, CHINESE_COLUMN_0, 2);
  HDG12864F1_WriteChineseCharUpper(cCharDuUpper, CHINESE_COLUMN_1, 2);
  HDG12864F1_WriteChineseCharLower(cCharDuLower, CHINESE_COLUMN_1, 2);

   
 

  HDG12864F1_WriteChineseCharUpper(cCharMoUpper, CHINESE_COLUMN_2, 6);
  HDG12864F1_WriteChineseCharLower(cCharMoLower, CHINESE_COLUMN_2, 6);
  HDG12864F1_WriteChineseCharUpper(cCharShi3Upper, CHINESE_COLUMN_3, 6);
  HDG12864F1_WriteChineseCharLower(cCharShi3Lower, CHINESE_COLUMN_3, 6);
}

void DrawSet() {
	HDG12864F1_WriteChineseCharUpper(cCharSheUpper, CHINESE_COLUMN_0, 6);
  	HDG12864F1_WriteChineseCharLower(cCharSheLower, CHINESE_COLUMN_0, 6);
	HDG12864F1_WriteChineseCharUpper(cCharZhi2Upper, CHINESE_COLUMN_1, 6);
  	HDG12864F1_WriteChineseCharLower(cCharZhi2Lower, CHINESE_COLUMN_1, 6);
}

void MainUi() {
	if (ui_mode == 0) {
		dht11_read_data(&temperature, &humidity); 

		HDG12864F1_WriteEnglishChar(Char_MaoHao, NUMBER_COLUMN_4, 0);  
  		HDG12864F1_WriteEnglishChar(Number0_9[temperature/10], NUMBER_COLUMN_5, 0);
		HDG12864F1_WriteEnglishChar(Number0_9[temperature%10], NUMBER_COLUMN_6, 0);
  		HDG12864F1_WriteChineseCharUpper(cCharSheShiDuUpper, CHINESE_COLUMN_4, 0);
  		HDG12864F1_WriteChineseCharLower(cCharSheShiDuLower, CHINESE_COLUMN_4, 0);
		HDG12864F1_WriteEnglishChar(Char_MaoHao, NUMBER_COLUMN_4, 2); 
    	HDG12864F1_WriteEnglishChar(Char_BaiFenHao, NUMBER_COLUMN_8, 2);  
  		HDG12864F1_WriteEnglishChar(Char_R, NUMBER_COLUMN_9, 2);  
		HDG12864F1_WriteEnglishChar(Char_H, NUMBER_COLUMN_A, 2);
  		HDG12864F1_WriteEnglishChar(Number0_9[humidity/10], NUMBER_COLUMN_5, 2);  
  		HDG12864F1_WriteEnglishChar(Number0_9[humidity%10], NUMBER_COLUMN_6, 2);
		HDG12864F1_WriteChineseCharUpper(cCharXianUpper, CHINESE_COLUMN_0, 6);
  		HDG12864F1_WriteChineseCharLower(cCharXianLower, CHINESE_COLUMN_0, 6);
  		HDG12864F1_WriteChineseCharUpper(cCharShi2Upper, CHINESE_COLUMN_1, 6);
		HDG12864F1_WriteChineseCharLower(cCharShi2Lower, CHINESE_COLUMN_1, 6);
	}
	if (ui_mode == 1) {
		DrawSet();
	}
}

void HAL_Delay_us(uint32_t nus)
{
    const uint32_t ticks = (SystemCoreClock / 1000000U) * nus;
    const uint32_t reload = SysTick->LOAD;
    uint32_t previous = SysTick->VAL;
    uint32_t elapsed = 0U;

    while (elapsed < ticks)
    {
        const uint32_t current = SysTick->VAL;
        if (previous >= current)
        {
            elapsed += previous - current;
        }
        else
        {
            elapsed += previous + (reload + 1U - current);
        }
        previous = current;
    }
}


void Delay_ms(uint16_t ms) {
    while(ms--) {
        HAL_Delay(1);  
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == ChangeButton_Pin) {
		ui_mode ++;
		ui_mode %= 2;
		ui_dirty = 1;
	}

}
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
