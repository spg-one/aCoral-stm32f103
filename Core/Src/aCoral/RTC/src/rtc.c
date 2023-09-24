#include "acoral.h"	    

RTC_HandleTypeDef RTC_Handler;  //RTC句柄

// _calendar_obj calendar;//时钟结构体 
uint32_t timestap; //时间戳

//实时时钟配置
//初始化RTC时钟,同时检测时钟是否工作正常
//BKP->DR1用于保存是否第一次配置的设置
//返回0:正常
//其他:错误代码
uint8_t RTC_Init(void)
{
	RTC_Handler.Instance=RTC; 
	RTC_Handler.Init.AsynchPrediv=32767; 	//时钟周期设置(有待观察,看是否跑慢了?)理论值：32767	
	if(HAL_RTC_Init(&RTC_Handler)!=HAL_OK) return 1;
	if(HAL_RTCEx_BKUPRead(&RTC_Handler,RTC_BKP_DR1)!=0X5053)//是否第一次配置，要改的话，和下面全部一起改
	{
		RTC_Set(2023,9,11,16,5,0); //设置日期和时间，2019年11月27日，18点08分0秒		 									  
		HAL_RTCEx_BKUPWrite(&RTC_Handler,RTC_BKP_DR1,0X5053);//标记已经初始化过了
	 	acoral_print("------------------FIRST SET TIME---------------------\n");
	}
	
	__HAL_RTC_ALARM_ENABLE_IT(&RTC_Handler,RTC_IT_SEC); 	//允许秒中断
	__HAL_RTC_ALARM_ENABLE_IT(&RTC_Handler,RTC_IT_ALRA); 	//允许闹钟中断
    HAL_NVIC_SetPriority(RTC_IRQn,0x01,0x02); 				//抢占优先级1,子优先级2
    HAL_NVIC_EnableIRQ(RTC_IRQn);	 
	
	RTC_Get();//更新时间 
	return 0; //ok
}

//RTC底层驱动，时钟配置
//此函数会被HAL_RTC_Init()调用
//hrtc:RTC句柄
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    __HAL_RCC_PWR_CLK_ENABLE();	//使能电源时钟PWR
	HAL_PWR_EnableBkUpAccess();	//取消备份区域写保护
	__HAL_RCC_BKP_CLK_ENABLE();	//使能BSP时钟
	
	RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_LSE;//LSE配置
    RCC_OscInitStruct.PLL.PLLState=RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState=RCC_LSE_ON;                  //RTC使用LSE
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    PeriphClkInitStruct.PeriphClockSelection=RCC_PERIPHCLK_RTC;//外设为RTC
    PeriphClkInitStruct.RTCClockSelection=RCC_RTCCLKSOURCE_LSE;//RTC时钟源为LSE
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
        
    __HAL_RCC_RTC_ENABLE();//RTC时钟使能
}

//RTC时钟中断
//每秒触发一次  	 
void RTC_IRQHandler(void)
{		 
	if(__HAL_RTC_ALARM_GET_FLAG(&RTC_Handler,RTC_FLAG_SEC)!=RESET)  //秒中断
	{
	 	__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_SEC); 		//清除秒中断
		RTC_Get();				//更新时间   											//LED1翻转
	}		

	if(__HAL_RTC_ALARM_GET_FLAG(&RTC_Handler,RTC_FLAG_SEC)!=RESET)  //闹钟中断
	{
		__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_ALRAF); 	//清除闹钟中断   
		RTC_Get();				//更新时间   
		printf("ALARM A!\r\n");
	}	
	__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_OW); 		//清除溢出  	    						 	   	 
}

//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//year:年份
//返回值:该年份是不是闰年.1,是.0,不是
uint8_t Is_Leap_Year(uint16_t year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	

//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.										 
//平年的月份日期表
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
//syear,smon,sday,hour,min,sec：年月日时分秒
//返回值：设置结果。0，成功；1，失败。
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;

	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数
		else seccount+=31536000;			  //平年的秒钟数
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //把前面月份的秒钟数相加
	{
		seccount+=(uint32_t)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数	   
	}
	seccount+=(uint32_t)(sday-1)*86400;//把前面日期的秒钟数相加 
	seccount+=(uint32_t)hour*3600;//小时秒钟数
    seccount+=(uint32_t)min*60;	 //分钟秒钟数
	seccount+=sec;//最后的秒钟加上去

	//设置时钟
    RCC->APB1ENR|=1<<28;//使能电源时钟
    RCC->APB1ENR|=1<<27;//使能备份时钟
	PWR->CR|=1<<8;    //取消备份区写保护
	//上面三步是必须的!
	RTC->CRL|=1<<4;   //允许配置 
	RTC->CNTL=seccount&0xffff;
	RTC->CNTH=seccount>>16;
	RTC->CRL&=~(1<<4);//配置更新
	while(!(RTC->CRL&(1<<5)));//等待RTC寄存器操作完成 
	
	RTC_Get();//设置完之后更新一下数据 	
	return 0;	    
}
//初始化闹钟		  
//以1970年1月1日为基准
//1970~2099年为合法年份
//syear,smon,sday,hour,min,sec：闹钟的年月日时分秒   
//返回值:0,成功;其他:错误代码.
uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数
		else seccount+=31536000;			  //平年的秒钟数
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //把前面月份的秒钟数相加
	{
		seccount+=(uint32_t)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数	   
	}
	seccount+=(uint32_t)(sday-1)*86400;//把前面日期的秒钟数相加 
	seccount+=(uint32_t)hour*3600;//小时秒钟数
    seccount+=(uint32_t)min*60;	 //分钟秒钟数
	seccount+=sec;//最后的秒钟加上去 			    
	//设置时钟
    RCC->APB1ENR|=1<<28;//使能电源时钟
    RCC->APB1ENR|=1<<27;//使能备份时钟
	PWR->CR|=1<<8;    //取消备份区写保护
	//上面三步是必须的!
	RTC->CRL|=1<<4;   //允许配置 
	RTC->ALRL=seccount&0xffff;
	RTC->ALRH=seccount>>16;
	RTC->CRL&=~(1<<4);//配置更新
	while(!(RTC->CRL&(1<<5)));//等待RTC寄存器操作完成  
	return 0;	    
}
//得到当前的时间，结果保存在calendar结构体里面
//返回值:0,成功;其他:错误代码.
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0; 
	uint32_t temp=0;
	uint16_t temp1=0;	  
 	timecount=RTC->CNTH;//得到计数器中的值(秒钟数)
	timecount<<=16;
	timecount+=RTC->CNTL;			 

	timestap = timecount;

	//时间戳转换
 	// temp=timecount/86400;   //得到天数(秒钟数对应的)
	// if(daycnt!=temp)//超过一天了
	// {	  
	// 	daycnt=temp;
	// 	temp1=1970;	//从1970年开始
	// 	while(temp>=365)
	// 	{				 
	// 		if(Is_Leap_Year(temp1))//是闰年
	// 		{
	// 			if(temp>=366)temp-=366;//闰年的秒钟数
	// 			else break;  
	// 		}
	// 		else temp-=365;	  //平年 
	// 		temp1++;  
	// 	}   
	// 	calendar.w_year=temp1;//得到年份
	// 	temp1=0;
	// 	while(temp>=28)//超过了一个月
	// 	{
	// 		if(Is_Leap_Year(calendar.w_year)&&temp1==1)//当年是不是闰年/2月份
	// 		{
	// 			if(temp>=29)temp-=29;//闰年的秒钟数
	// 			else break; 
	// 		}
	// 		else 
	// 		{
	// 			if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
	// 			else break;
	// 		}
	// 		temp1++;  
	// 	}
	// 	calendar.w_month=temp1+1;	//得到月份
	// 	calendar.w_date=temp+1;  	//得到日期 
	// }
	// temp=timecount%86400;     		//得到秒钟数   	   
	// calendar.hour=temp/3600;     	//小时
	// calendar.min=(temp%3600)/60; 	//分钟	
	// calendar.sec=(temp%3600)%60; 	//秒钟
	
	return 0;
}	 
















