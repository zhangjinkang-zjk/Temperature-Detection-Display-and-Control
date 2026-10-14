#ifndef __HDG12864F1_H
#define __HDG12864F1_H

#include "stm32f1xx_hal.h"

#define DISPLAY_LEFT_TO_RIGHT 1
/*********汉字列地址*********/
#define CHINESE_COLUMN_0  0
#define CHINESE_COLUMN_1  16
#define CHINESE_COLUMN_2  32
#define CHINESE_COLUMN_3  48
#define CHINESE_COLUMN_4  64
#define CHINESE_COLUMN_5  80
#define CHINESE_COLUMN_6  96
#define CHINESE_COLUMN_7  112  
/********数字字符列地址******/
#define NUMBER_COLUMN_0  0
#define NUMBER_COLUMN_1  8
#define NUMBER_COLUMN_2  16
#define NUMBER_COLUMN_3  24
#define NUMBER_COLUMN_4  32
#define NUMBER_COLUMN_5  40
#define NUMBER_COLUMN_6  48
#define NUMBER_COLUMN_7  56  
#define NUMBER_COLUMN_8  64
#define NUMBER_COLUMN_9  72
#define NUMBER_COLUMN_A  80
#define NUMBER_COLUMN_B  88
#define NUMBER_COLUMN_C  96  
#define NUMBER_COLUMN_D  104  
#define NUMBER_COLUMN_E  112  
#define NUMBER_COLUMN_F  120  
 
extern uint8_t cCharZhiUpper[16];
extern uint8_t cCharZhiLower[16];
extern uint8_t cCharNengUpper[16];
extern uint8_t cCharNengLower[16];
extern uint8_t cCharJiaUpper[16];
extern uint8_t cCharJiaLower[16];
extern uint8_t cCharJuUpper[16];
extern uint8_t cCharJuLower[16];
extern uint8_t cCharWenUpper[16];
extern uint8_t cCharWenLower[16];
extern uint8_t cCharDuUpper[16];
extern uint8_t cCharDuLower[16];
extern uint8_t cCharShiUpper[16]; 
extern uint8_t cCharShiLower[16];
extern uint8_t cCharSheShiDuUpper[16];
extern uint8_t cCharSheShiDuLower[16];
extern uint8_t Char_R[16];
extern uint8_t Char_H[16];
extern uint8_t Char_BaiFenHao[16];
extern uint8_t Char_MaoHao[16];
extern uint8_t Number0_9[10][16];

void HDG12864F1_Init(void);
void Lcd_GPIO_Configuration(void);
void HDG12864F1_SetColumnAddress(uint8_t address);
void HDG12864F1_SetPageAddress(uint8_t pageAddress);
void HDG12864F1_SetStartLine(uint8_t line);
void HDG12864F1_WriteData(uint8_t data_);
void wrt_cmd(uint8_t command);
void wrt_dt(uint8_t data_);
void HDG12864F1_WriteEnglishChar(uint8_t *pEChar, uint8_t column, uint8_t page);
void HDG12864F1_Direction(uint8_t direction);  
void HDG12864F1_Clear(void);
void HDG12864F1_WriteChineseCharUpper(uint8_t *pCChar, uint8_t column, uint8_t page);
void HDG12864F1_WriteChineseCharLower(uint8_t *pCChar, uint8_t column, uint8_t page);


extern uint8_t cCharXianUpper[16];
extern uint8_t cCharXianLower[16];
extern uint8_t cCharShi2Upper[16];
extern uint8_t cCharShi2Lower[16];
extern uint8_t cCharMoUpper[16];
extern uint8_t cCharMoLower[16];
extern uint8_t cCharShi3Upper[16];
extern uint8_t cCharShi3Lower[16];
extern uint8_t cCharSheUpper[16];
extern uint8_t cCharSheLower[16];
extern uint8_t cCharZhi2Upper[16];
extern uint8_t cCharZhi2Lower[16];
#endif /* __HDG12864F1_H */
