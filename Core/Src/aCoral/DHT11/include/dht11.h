/*** 
 * @Author: jp
 * @Date: 2023-06-13 08:01:43
 * @LastEditTime: 2023-06-14 14:57:05
 * @LastEditors: jp
 * @Description: 
 * @FilePath: \aCoral-stm32f103-master\Core\Src\aCoral\DHT11\include\dht11.h
 */

#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f1xx_hal.h" 
#include "acoral.h"


#define DHT_GPIO_PORT	GPIOA
#define DHT_GPIO_PIN	GPIO_PIN_11

extern uint8_t DHT_Buffer[5];
extern uint8_t j;

void DHT_GPIO_Init(uint32_t Mode);
uint8_t DHT_Start(void);
uint8_t DHT_Get_Byte_Data(void);
uint8_t DHT_Get_Temp_Humi_Data(uint8_t buffer[]);
void for_delay_us(uint32_t us);
void get_temp_humi_thread();


#endif
