#ifndef __OLED_H
#define __OLED_H
// #include "stm32f1xx_hal.h"
#include "sys.h"

//OLED模式设置
//0: 4线串行模式  （模块的BS1，BS2均接GND）
//1: 并行8080模式 （模块的BS1，BS2均接VCC）
#define OLED_MODE 	1
		    						  
//-----------------OLED端口定义----------------  		
#define OLED_CS     PCout(9)
//#define OLED_RST  PGout(15)//在MINISTM32上直接接到了STM32的复位脚！
#define OLED_RS     PCout(8)
#define OLED_WR     PCout(7)
#define OLED_RD     PCout(6)
 
//PB0~7,作为数据线
#define DATAOUT(x) GPIOB->ODR=(GPIOB->ODR&0xff00)|(x&0x00FF); //输出

//使用4线串行接口时使用 
#define OLED_SCLK   PBout(0)
#define OLED_SDIN   PBout(1)

#define OLED_CMD  	0		//写命令
#define OLED_DATA 	1		//写数据

//OLED控制用函数
void OLED_WR_Byte(uint8_t dat,uint8_t cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);		   
							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t size);
#endif
