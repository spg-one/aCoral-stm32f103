#ifndef __RTC_H
#define __RTC_H	  


extern RTC_HandleTypeDef RTC_Handler;  //RTC句柄
												    
//时间结构体
// typedef struct 
// {
// 	uint8_t hour;
// 	uint8_t min;
// 	uint8_t sec;			
// 	//公历日月年周
// 	uint16_t w_year;
// 	uint8_t  w_month;
// 	uint8_t  w_date;
// }_calendar_obj;					 

// extern _calendar_obj calendar;				//日历结构体
extern uint32_t timestap;


uint8_t RTC_Init(void);        					//初始化RTC,返回0,失败;1,成功;
uint8_t Is_Leap_Year(uint16_t year);					//平年,闰年判断
uint8_t RTC_Get(void);         					//获取时间   
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);		//设置时间	
uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);	//设置闹钟	
#endif



















