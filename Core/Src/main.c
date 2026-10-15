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
#define IIC_SCL_GPIO_PORT               GPIOB
#define IIC_SCL_GPIO_PIN                GPIO_PIN_6
#define IIC_SCL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

#define IIC_SDA_GPIO_PORT               GPIOB
#define IIC_SDA_GPIO_PIN                GPIO_PIN_7
#define IIC_SDA_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

/* IO操作 */
#define IIC_SCL(x)        do{ x ? \
                              HAL_GPIO_WritePin(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                          }while(0)       /* SCL */

#define IIC_SDA(x)        do{ x ? \
                              HAL_GPIO_WritePin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                          }while(0)       /* SDA */

#define IIC_READ_SDA     HAL_GPIO_ReadPin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN) /* 读取SDA */

/* 存储器24CXX型号容量定义 */
#define AT24C01     127
#define AT24C02     255
#define AT24C04     511
#define AT24C08     1023
#define AT24C16     2047
#define AT24C32     4095
#define AT24C64     8191
#define AT24C128    16383
#define AT24C256    32767
/* 开发板使用的是24c02，所以定义EE_TYPE为AT24C02 */
#define EE_TYPE     AT24C02
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
void at24cxx_init(void);        /* 初始化IIC */
uint8_t at24cxx_check(void);    /* 检查器件 */
uint8_t at24cxx_read_one_byte(uint16_t addr);                       /* 指定地址读取一个字节 */
void at24cxx_write_one_byte(uint16_t addr,uint8_t data);            /* 指定地址写入一个字节 */
void at24cxx_write(uint16_t addr, uint8_t *pbuf, uint16_t datalen); /* 从指定地址开始写入指定长度的数据 */
void at24cxx_read(uint16_t addr, uint8_t *pbuf, uint16_t datalen);  /* 从指定地址开始读出指定长度的数据 */
void HAL_Delay_us(uint32_t nus); 
/* IIC所有操作函数声明 */
void iic_init(void);            /* 初始化IIC的IO口 */
void iic_start(void);           /* 发送IIC开始信号 */
void iic_stop(void);            /* 发送IIC停止信号 */
void iic_ack(void);             /* IIC发送ACK信号 */
void iic_nack(void);            /* IIC不发送ACK信号 */
uint8_t iic_wait_ack(void);     /* IIC等待ACK信号 */
void iic_send_byte(uint8_t txd);/* IIC发送一个字节 */
uint8_t iic_read_byte(unsigned char ack);/* IIC读取一个字节 */
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

void iic_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    IIC_SCL_GPIO_CLK_ENABLE();  /* SCL引脚时钟使能 */
    IIC_SDA_GPIO_CLK_ENABLE();  /* SDA引脚时钟使能 */

    gpio_init_struct.Pin = IIC_SCL_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;        /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 高速 */
    HAL_GPIO_Init(IIC_SCL_GPIO_PORT, &gpio_init_struct);/* SCL */

    gpio_init_struct.Pin = IIC_SDA_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_OD;        /* 开漏输出 */
    HAL_GPIO_Init(IIC_SDA_GPIO_PORT, &gpio_init_struct);/* SDA */
    /* SDA引脚模式设置,开漏输出,上拉, 这样就不用再设置IO方向了, 开漏输出的时候(=1), 也可以读取外部信号的高低电平 */

    iic_stop();     /* 停止总线上所有设备 */
}

/**
 * @brief       IIC延时函数,用于控制IIC读写速度
 * @param       无
 * @retval      无
 */
static void iic_delay(void)
{
//  delay_us(2);    /* 2us的延时, 读写速度在250Khz以内 */
    HAL_Delay_us(2); 
}

/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void iic_start(void)
{
    IIC_SDA(1);
    IIC_SCL(1);
    iic_delay();
    IIC_SDA(0);     /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    iic_delay();
    IIC_SCL(0);     /* 钳住I2C总线，准备发送或接收数据 */
    iic_delay();
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void iic_stop(void)
{
    IIC_SDA(0);     /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    iic_delay();
    IIC_SCL(1);
    iic_delay();
    IIC_SDA(1);     /* 发送I2C总线结束信号 */
    iic_delay();
}

/**
 * @brief       等待应答信号到来
 * @param       无
 * @retval      1，接收应答失败
 *              0，接收应答成功
 */
uint8_t iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    IIC_SDA(1);     /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    iic_delay();
    IIC_SCL(1);     /* SCL=1, 此时从机可以返回ACK */
    iic_delay();

    while (IIC_READ_SDA)    /* 等待应答 */
    {
        waittime++;

        if (waittime > 250)
        {
            iic_stop();
            rack = 1;
            break;
        }
    }

    IIC_SCL(0);     /* SCL=0, 结束ACK检查 */
    iic_delay();
    return rack;
}

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void iic_ack(void)
{
    IIC_SDA(0);     /* SCL 0 -> 1  时 SDA = 0,表示应答 */
    iic_delay();
    IIC_SCL(1);     /* 产生一个时钟 */
    iic_delay();
    IIC_SCL(0);
    iic_delay();
    IIC_SDA(1);     /* 主机释放SDA线 */
    iic_delay();
}

/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void iic_nack(void)
{
    IIC_SDA(1);     /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    iic_delay();
    IIC_SCL(1);     /* 产生一个时钟 */
    iic_delay();
    IIC_SCL(0);
    iic_delay();
}

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 */
void iic_send_byte(uint8_t data)
{
    uint8_t t;
    
    for (t = 0; t < 8; t++)
    {
        IIC_SDA((data & 0x80) >> 7);    /* 高位先发送 */
        iic_delay();
        IIC_SCL(1);
        iic_delay();
        IIC_SCL(0);
        data <<= 1;     /* 左移1位,用于下一次发送 */
    }
    IIC_SDA(1);         /* 发送完成, 主机释放SDA线 */
}

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t iic_read_byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
    {
        receive <<= 1;  /* 高位先输出,所以先收到的数据位要左移 */
        IIC_SCL(1);
        iic_delay();

        if (IIC_READ_SDA)
        {
            receive++;
        }
        
        IIC_SCL(0);
        iic_delay();
    }

    if (!ack)
    {
        iic_nack();     /* 发送nACK */
    }
    else
    {
        iic_ack();      /* 发送ACK */
    }

    return receive;
}

/**
 * @brief       初始化IIC接口
 * @param       无
 * @retval      无
 */
void at24cxx_init(void)
{
    iic_init();
}

/**
 * @brief       在AT24CXX指定地址读出一个数据
 * @param       readaddr: 开始读数的地址
 * @retval      读到的数据
 */
uint8_t at24cxx_read_one_byte(uint16_t addr)
{
    uint8_t temp = 0;
    iic_start();                /* 发送起始信号 */

    /* 根据不同的24CXX型号, 发送高位地址
     * 1, 24C16以上的型号, 分2个字节发送地址
     * 2, 24C16及以下的型号, 分1个低字节地址 + 占用器件地址的bit1~bit3位 用于表示高位地址, 最多11位地址
     *    对于24C01/02, 其器件地址格式(8bit)为: 1  0  1  0  A2  A1  A0  R/W
     *    对于24C04,    其器件地址格式(8bit)为: 1  0  1  0  A2  A1  a8  R/W
     *    对于24C08,    其器件地址格式(8bit)为: 1  0  1  0  A2  a9  a8  R/W
     *    对于24C16,    其器件地址格式(8bit)为: 1  0  1  0  a10 a9  a8  R/W
     *    R/W      : 读/写控制位 0,表示写; 1,表示读;
     *    A0/A1/A2 : 对应器件的1,2,3引脚(只有24C01/02/04/8有这些脚)
     *    a8/a9/a10: 对应存储整列的高位地址, 11bit地址最多可以表示2048个位置,可以寻址24C16及以内的型号
     */    
    if (EE_TYPE > AT24C16)      /* 24C16以上的型号, 分2个字节发送地址 */
    {
        iic_send_byte(0XA0);    /* 发送写命令, IIC规定最低位是0, 表示写入 */
        iic_wait_ack();         /* 每次发送完一个字节,都要等待ACK */
        iic_send_byte(addr >> 8);/* 发送高字节地址 */
    }
    else 
    {
        iic_send_byte(0XA0 + ((addr >> 8) << 1));   /* 发送器件 0XA0 + 高位a8/a9/a10地址,写数据 */
    }
    
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(addr % 256);  /* 发送低位地址 */
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    
    iic_start();                /* 重新发送起始信号 */ 
    iic_send_byte(0XA1);        /* 进入接收模式, IIC规定最低位是0, 表示读取 */
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    temp = iic_read_byte(0);    /* 接收一个字节数据 */
    iic_stop();                 /* 产生一个停止条件 */
    return temp;
}

/**
 * @brief       在AT24CXX指定地址写入一个数据
 * @param       addr: 写入数据的目的地址
 * @param       data: 要写入的数据
 * @retval      无
 */
void at24cxx_write_one_byte(uint16_t addr, uint8_t data)
{
    /* 原理说明见:at24cxx_read_one_byte函数, 本函数完全类似 */
    iic_start();                /* 发送起始信号 */

    if (EE_TYPE > AT24C16)      /* 24C16以上的型号, 分2个字节发送地址 */
    {
        iic_send_byte(0XA0);    /* 发送写命令, IIC规定最低位是0, 表示写入 */
        iic_wait_ack();         /* 每次发送完一个字节,都要等待ACK */
        iic_send_byte(addr >> 8);/* 发送高字节地址 */
    }
    else 
    {
        iic_send_byte(0XA0 + ((addr >> 8) << 1));   /* 发送器件 0XA0 + 高位a8/a9/a10地址,写数据 */
    }
    
    iic_wait_ack();             /* 每次发送完一个字节,都要等待ACK */
    iic_send_byte(addr % 256);  /* 发送低位地址 */
    iic_wait_ack();             /* 等待ACK, 此时地址发送完成了 */
    
    /* 因为写数据的时候,不需要进入接收模式了,所以这里不用重新发送起始信号了 */
    iic_send_byte(data);        /* 发送1字节 */
    iic_wait_ack();             /* 等待ACK */
    iic_stop();                 /* 产生一个停止条件 */
//  delay_ms(10);               /* 注意: EEPROM 写入比较慢,必须等到10ms后再写下一个字节 */
    HAL_Delay_us(10000); 
}
 
/**
 * @brief       检查AT24CXX是否正常
 *   @note      检测原理: 在器件的末地址写如0X55, 然后再读取, 如果读取值为0X55
 *              则表示检测正常. 否则,则表示检测失败.
 *
 * @param       无
 * @retval      检测结果
 *              0: 检测成功
 *              1: 检测失败
 */
uint8_t at24cxx_check(void)
{
    uint8_t temp;
    uint16_t addr = EE_TYPE;
    temp = at24cxx_read_one_byte(addr); /* 避免每次开机都写AT24CXX */

    if (temp == 0X55)   /* 读取数据正常 */
    {
        return 0;
    }
    else    /* 排除第一次初始化的情况 */
    {
        at24cxx_write_one_byte(addr, 0X55); /* 先写入数据 */
        temp = at24cxx_read_one_byte(255);  /* 再读取数据 */

        if (temp == 0X55)return 0;
    }

    return 1;
}

/**
 * @brief       在AT24CXX里面的指定地址开始读出指定个数的数据
 * @param       addr    : 开始读出的地址 对24c02为0~255
 * @param       pbuf    : 数据数组首地址
 * @param       datalen : 要读出数据的个数
 * @retval      无
 */
void at24cxx_read(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
    while (datalen--)
    {
        *pbuf++ = at24cxx_read_one_byte(addr++);
    }
}

/**
 * @brief       在AT24CXX里面的指定地址开始写入指定个数的数据
 * @param       addr    : 开始写入的地址 对24c02为0~255
 * @param       pbuf    : 数据数组首地址
 * @param       datalen : 要写入数据的个数
 * @retval      无
 */
void at24cxx_write(uint16_t addr, uint8_t *pbuf, uint16_t datalen)
{
    while (datalen--)
    {
        at24cxx_write_one_byte(addr, *pbuf);
        addr++;
        pbuf++;
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
