/*
 * @Author: jp
 * @Date: 2023-06-13 08:01:43
 * @LastEditTime: 2023-06-14 16:07:09
 * @LastEditors: jp
 * @Description: 
 * @FilePath: \aCoral-stm32f103-master\Core\Src\aCoral\DHT11\src\dht11.c
 */
#include "dht11.h"
extern void get_distance_thread();
// TIM7 CLock Params
extern TIM_HandleTypeDef htim7;
uint8_t TIM7_WaitSignal = 0;


void tim7_delay_us(uint16_t delay){
	if(TIM7_WaitSignal) {
		return;
	}
	TIM7_WaitSignal = 0;
	htim7.Instance -> CNT = 0x00;
  htim7.Init.Period = (uint32_t)delay;
	htim7.Instance -> ARR = htim7.Init.Period - 1;
	HAL_TIM_Base_Start_IT(&htim7);
	while(!TIM7_WaitSignal);
	HAL_TIM_Base_Stop_IT(&htim7);
	TIM7_WaitSignal = 0;
}

void TIM7_PeriodElapsedCallback() {
	if(!TIM7_WaitSignal) {
		TIM7_WaitSignal = 1;
	}
}



/**
  * @brief  DHT11_GPIO初始化函数
  * @param  Mode：指定输入或输出模式
  * @return None
  */
uint8_t DHT_Buffer[5];
uint8_t j = 0;
void DHT_GPIO_Init(uint32_t Mode)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟

    
    GPIO_Initure.Pin=DHT_GPIO_PIN;          //PA4
    GPIO_Initure.Mode=Mode;                 //输入输出模式
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//高速
    HAL_GPIO_Init(DHT_GPIO_PORT,&GPIO_Initure);
    
    HAL_GPIO_WritePin(DHT_GPIO_PORT,DHT_GPIO_PIN,GPIO_PIN_SET);//默认输出高电平 
    
}

/**
  * @brief  DHT11模块起始信号函数
  * @param  None
  * @return 1或0，标志起动信号成功与否
  */
uint8_t DHT_Start(void)
{
    DHT_GPIO_Init(GPIO_MODE_OUTPUT_OD);                     //输出模式
    
    HAL_GPIO_WritePin(DHT_GPIO_PORT,DHT_GPIO_PIN,GPIO_PIN_RESET);     //输出20ms低电平后拉高
    HAL_Delay(20);
    HAL_GPIO_WritePin(DHT_GPIO_PORT,DHT_GPIO_PIN,GPIO_PIN_SET);
    
    DHT_GPIO_Init(GPIO_MODE_INPUT);
    tim7_delay_us(20);
    
    if(!HAL_GPIO_ReadPin(DHT_GPIO_PORT, DHT_GPIO_PIN))
    {
        while(!HAL_GPIO_ReadPin(DHT_GPIO_PORT, DHT_GPIO_PIN));
        while(HAL_GPIO_ReadPin(DHT_GPIO_PORT, DHT_GPIO_PIN));
        return 1;
    }
    return 0;
   
}

/**
  * @brief  接收DHT11发送来8位的数据
  * @param  None
  * @return 返回接收到的8位数据
  */
uint8_t DHT_Get_Byte_Data(void)
{
	uint8_t temp = 0;
	for(uint8_t i = 0; i < 8; i++)
	{
		temp <<= 1;
		while(!HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_GPIO_PIN));
		tim7_delay_us(28);
		HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_GPIO_PIN) ? (temp |= 0x01) : (temp &= ~0x01);
		while(HAL_GPIO_ReadPin(DHT_GPIO_PORT,DHT_GPIO_PIN));
	}
	return temp;
}

/**
  * @brief  获取DHT11的温度湿度数据
  * @param  buffer[]：需要传入一个存储数据的数组
  * @return 返回数据校验是否正确 1：正确 0：失败
  */
uint8_t DHT_Get_Temp_Humi_Data(uint8_t buffer[])
{
	if(DHT_Start())
	{
		buffer[0] = DHT_Get_Byte_Data();
		buffer[1] = DHT_Get_Byte_Data();
		buffer[2] = DHT_Get_Byte_Data();
		buffer[3] = DHT_Get_Byte_Data();
		buffer[4] = DHT_Get_Byte_Data();
	}
	return (buffer[0]+buffer[1]+buffer[2]+buffer[3] == buffer[4]) ? 1 : 0;
}

/*
for循环实现延时us
*/
void for_delay_us(uint32_t us)
{
    uint32_t Delay = us * 8/4;
    do
    {
        __NOP();
    }
    while (Delay --);
}

/**
 * @brief: 线程函数
 * @return {*}
 */
void get_temp_humi_thread()
{
  if((cur_period.significant&(0x01u<<1)))
  {
    acoral_enter_critical();
    ((period_private_data_t *)acoral_cur_thread->private_data)->time = (cur_period.temp_humi)*1000;
    // period_thread_delay(acoral_cur_thread,((period_private_data_t *)acoral_cur_thread->private_data)->time);
    cur_period.significant &= (~(0x01u<<1));
    acoral_exit_critical();
  }
  else
  {
	  if(DHT_Get_Temp_Humi_Data(DHT_Buffer))
    {
      // acoral_print("Temp:%d.%d    ",DHT_Buffer[2],DHT_Buffer[3]);
      // acoral_print("Humi:%d.%d\r\n",DHT_Buffer[0],DHT_Buffer[1]);
      Buffer.humi_int = DHT_Buffer[0];
      Buffer.humi_dec= DHT_Buffer[1];
      Buffer.temp_int = DHT_Buffer[2];
      Buffer.temp_dec = DHT_Buffer[3];
      Buffer.temp_humi_period = (((period_private_data_t *)acoral_cur_thread->private_data)->time)/1000;
      data_ready|=(1<<1);
    }
    else
    {
      acoral_print("-------\r\n");
    }
  }
}

uint8_t get_newest_temp(void) {
  return DHT_Buffer[2];
}