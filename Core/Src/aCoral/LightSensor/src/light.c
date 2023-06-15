#include "light.h"

/*
    模块在环境光线不达到阈值时，DO输出高电平
    超过设定阈值时，DO输出低电平
*/
void LightSensor_Init(void)
{
     GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟
    GPIO_Initure.Pin=GPIO_PIN_0;            //PA0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull=GPIO_PULLDOWN;        //下拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//高速
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
}

void LightSensor_thread(void)
{
    // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    // if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
    // if(i%2 == 0)
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
    {
        // LED0=1;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
    }else
    {
        // LED0=0;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
    }
}