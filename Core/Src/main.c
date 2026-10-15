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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dht11.h"
#include "HDG12864F1.h"
#include "at24cxx.h"
#include "stm32f1xx_hal_tim.h"
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADDRTM 0
#define ADDRTL 1
#define ADDRHM 2
#define ADDRHL 3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* 要写入到 24C02 的字符串数据 */
volatile uint8_t ui_mode = 0;
volatile uint8_t ui_dirty = 2;
volatile uint8_t ee_save = 0;
uint8_t temperature = 0;
uint8_t humidity = 0;
volatile uint8_t temmost = 40;
volatile uint8_t temleast = 0;
volatile uint8_t hummost = 70;
volatile uint8_t humleast = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_Delay_us(uint32_t nus);
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
  Lcd_GPIO_Configuration();
  HDG12864F1_Init();          //OLED功能初始化
  HDG12864F1_Clear();         //清屏
  HAL_Delay_us(10);
  at24cxx_init();  
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
                                   /* 初始化24CXX */
  if (at24cxx_read_one_byte(ADDRTM) == 0xFF)
  {
    at24cxx_write_one_byte(ADDRTM, temmost);
    at24cxx_write_one_byte(ADDRTL, temleast);
    at24cxx_write_one_byte(ADDRHM, hummost);
    at24cxx_write_one_byte(ADDRHL, humleast);
  }
  else
  {
    temmost = at24cxx_read_one_byte(ADDRTM);
    temleast = at24cxx_read_one_byte(ADDRTL);
    hummost = at24cxx_read_one_byte(ADDRHM);
    humleast = at24cxx_read_one_byte(ADDRHL);
  }
  dht11_read_data(&temperature, &humidity);
  
  ui_dirty = 2;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	uint8_t d = ui_dirty;
	ui_dirty = 0;

	dht11_read_data(&temperature, &humidity);
	HDG12864F1_WriteEnglishChar(Number0_9[temperature/10], NUMBER_COLUMN_5, 0);  
	HDG12864F1_WriteEnglishChar(Number0_9[temperature%10], NUMBER_COLUMN_6, 0);

	HDG12864F1_WriteEnglishChar(Number0_9[humidity/10], NUMBER_COLUMN_5, 2);
	HDG12864F1_WriteEnglishChar(Number0_9[humidity%10], NUMBER_COLUMN_6, 2);
	if (ee_save) {
		ee_save = 0;
		at24cxx_write_one_byte(ADDRTM, temmost);
		at24cxx_write_one_byte(ADDRTL, temleast);
		at24cxx_write_one_byte(ADDRHM, hummost);
		at24cxx_write_one_byte(ADDRHL, humleast);
	}

	if (d == 2) {
		HDG12864F1_Clear();
		DrawStatic(ui_mode);
	}
	if (d != 0) {
		DrawValue(ui_mode);
	}

	if (temperature >= temmost) {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 100);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
	}
	else {
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
	}
	if (temperature <= temleast) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
	}
	else {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
	}
	Delay_ms(200);
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
void DrawStatic(uint8_t mode) {
  HDG12864F1_WriteChineseCharUpper(cCharWenUpper, CHINESE_COLUMN_0, 0);
  HDG12864F1_WriteChineseCharLower(cCharWenLower, CHINESE_COLUMN_0, 0);

  HDG12864F1_WriteChineseCharUpper(cCharDuUpper, CHINESE_COLUMN_1, 0);
  HDG12864F1_WriteChineseCharLower(cCharDuLower, CHINESE_COLUMN_1, 0);

  HDG12864F1_WriteEnglishChar(Char_MaoHao, NUMBER_COLUMN_4, 0);
  HDG12864F1_WriteChineseCharUpper(cCharSheShiDuUpper, CHINESE_COLUMN_4, 0);
  HDG12864F1_WriteChineseCharLower(cCharSheShiDuLower, CHINESE_COLUMN_4, 0);

  HDG12864F1_WriteChineseCharUpper(cCharShiUpper, CHINESE_COLUMN_0, 2);
  HDG12864F1_WriteChineseCharLower(cCharShiLower, CHINESE_COLUMN_0, 2);

  HDG12864F1_WriteChineseCharUpper(cCharDuUpper, CHINESE_COLUMN_1, 2);
  HDG12864F1_WriteChineseCharLower(cCharDuLower, CHINESE_COLUMN_1, 2);

  HDG12864F1_WriteEnglishChar(Char_MaoHao, NUMBER_COLUMN_4, 2);
  HDG12864F1_WriteEnglishChar(Char_BaiFenHao, NUMBER_COLUMN_8, 2);
  HDG12864F1_WriteEnglishChar(Char_R, NUMBER_COLUMN_9, 2);
  HDG12864F1_WriteEnglishChar(Char_H, NUMBER_COLUMN_A, 2);

  if (mode == 0) {
    HDG12864F1_WriteChineseCharUpper(cCharXianUpper, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharLower(cCharXianLower, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharUpper(cCharShi2Upper, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharLower(cCharShi2Lower, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharUpper(cCharMoUpper, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharLower(cCharMoLower, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharUpper(cCharShi3Upper, CHINESE_COLUMN_3, 6);
    HDG12864F1_WriteChineseCharLower(cCharShi3Lower, CHINESE_COLUMN_3, 6);
  }
  if (mode == 1) {
    HDG12864F1_WriteChineseCharUpper(cCharWenUpper, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharLower(cCharWenLower, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharUpper(cCharDuUpper, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharLower(cCharDuLower, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharUpper(cCharShangUpper, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharLower(cCharShangLower, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharUpper(cCharXian2Upper, CHINESE_COLUMN_3, 6);
    HDG12864F1_WriteChineseCharLower(cCharXian2Lower, CHINESE_COLUMN_3, 6);
  }
  if (mode == 2) {
    HDG12864F1_WriteChineseCharUpper(cCharWenUpper, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharLower(cCharWenLower, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharUpper(cCharDuUpper, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharLower(cCharDuLower, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharUpper(cCharXiaUpper, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharLower(cCharXiaLower, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharUpper(cCharXian2Upper, CHINESE_COLUMN_3, 6);
    HDG12864F1_WriteChineseCharLower(cCharXian2Lower, CHINESE_COLUMN_3, 6);
  }
  if (mode == 3) {
    HDG12864F1_WriteChineseCharUpper(cCharShiUpper, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharLower(cCharShiLower, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharUpper(cCharDuUpper, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharLower(cCharDuLower, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharUpper(cCharShangUpper, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharLower(cCharShangLower, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharUpper(cCharXian2Upper, CHINESE_COLUMN_3, 6);
    HDG12864F1_WriteChineseCharLower(cCharXian2Lower, CHINESE_COLUMN_3, 6);
  }
  if (mode == 4) {
    HDG12864F1_WriteChineseCharUpper(cCharShiUpper, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharLower(cCharShiLower, CHINESE_COLUMN_0, 6);
    HDG12864F1_WriteChineseCharUpper(cCharDuUpper, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharLower(cCharDuLower, CHINESE_COLUMN_1, 6);
    HDG12864F1_WriteChineseCharUpper(cCharXiaUpper, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharLower(cCharXiaLower, CHINESE_COLUMN_2, 6);
    HDG12864F1_WriteChineseCharUpper(cCharXian2Upper, CHINESE_COLUMN_3, 6);
    HDG12864F1_WriteChineseCharLower(cCharXian2Lower, CHINESE_COLUMN_3, 6);
  }
}

void DrawValue(uint8_t mode) {
  HDG12864F1_WriteEnglishChar(Number0_9[temperature/10], NUMBER_COLUMN_5, 0);
  HDG12864F1_WriteEnglishChar(Number0_9[temperature%10], NUMBER_COLUMN_6, 0);
  HDG12864F1_WriteEnglishChar(Number0_9[humidity/10], NUMBER_COLUMN_5, 2);
  HDG12864F1_WriteEnglishChar(Number0_9[humidity%10], NUMBER_COLUMN_6, 2);

  if (mode == 1) {
    HDG12864F1_WriteEnglishChar(Number0_9[temmost/10], NUMBER_COLUMN_9, 6);
    HDG12864F1_WriteEnglishChar(Number0_9[temmost%10], NUMBER_COLUMN_A, 6);
  }
  if (mode == 2) {
    HDG12864F1_WriteEnglishChar(Number0_9[temleast/10], NUMBER_COLUMN_9, 6);
    HDG12864F1_WriteEnglishChar(Number0_9[temleast%10], NUMBER_COLUMN_A, 6);
  }
  if (mode == 3) {
    HDG12864F1_WriteEnglishChar(Number0_9[hummost/10], NUMBER_COLUMN_9, 6);
    HDG12864F1_WriteEnglishChar(Number0_9[hummost%10], NUMBER_COLUMN_A, 6);
  }
  if (mode == 4) {
    HDG12864F1_WriteEnglishChar(Number0_9[humleast/10], NUMBER_COLUMN_9, 6);
    HDG12864F1_WriteEnglishChar(Number0_9[humleast%10], NUMBER_COLUMN_A, 6);
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == ChangeButton_Pin) {
		ui_mode = (ui_mode + 1) % 5;
		ui_dirty = 2;
		return;
	}

	if (ui_mode == 0) return;

	if (GPIO_Pin == AddButton_Pin) {
		if (ui_mode == 1)      { if (temmost < 99) temmost++; }
		else if (ui_mode == 2) { if (temleast < 99) temleast++; }
		else if (ui_mode == 3) { if (hummost < 99) hummost++; }
		else if (ui_mode == 4) { if (humleast < 99) humleast++; }
		else return;
	}
	else if (GPIO_Pin == MinusButton_Pin) {
		if (ui_mode == 1)      { if (temmost > 0) temmost--; }
		else if (ui_mode == 2) { if (temleast > 0) temleast--; }
		else if (ui_mode == 3) { if (hummost > 0) hummost--; }
		else if (ui_mode == 4) { if (humleast > 0) humleast--; }
		else return;
	}
	else return;



	ui_dirty = 1;
	ee_save = 1;

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
