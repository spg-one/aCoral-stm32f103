/**
* @file: lora.c
* @author: 王若宇
* @brief: 主机从机服务函数的定义，简单实现主机不断发送测试数据，从机接收后串口打印数据
* @version: 1.0
* @date: 2023-05-13
*/
#include"lora.h"

tRadioDriver *Radio = NULL;                                    
uint8_t  Buffer[BUFFER_SIZE];                
uint8_t EnableMaster = true;

uint8_t MY_TEST_Msg[] = "hello";        //测试数据

/**
 * @brief Radio初始化
 * @param None  
 * @return void 
 */
void lora_init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    Radio = RadioDriverInit( );	//Radio初始化，主要是针对sx1232、sx1272、sx1276不同芯片进行驱动选择，sx1278和sx1276使用相同驱动，在paltform.h中进行选择设置
    Radio->Init( );				//sx1278的真正初始化，根据用户在radio.h中设置的LORA变量选择进行lora初始化还是fsk初始化
}

/**
 * @brief 主机服务函数master，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void master(void *args)
{
    if(data_ready)
    {
        // Buffer[5] = data_ready;
        // Radio->SetTxPacket( Buffer, sizeof(Buffer) ); 
        // Radio->Process();
        // Radio->Process();
        // Radio->Process();
        // acoral_print("Distance: %d cm\r\nTemp:%d.%d    Humi:%d.%d\r\nsignificant bit:%d", Buffer[0],Buffer[3],Buffer[4],Buffer[1],Buffer[2],Buffer[5]);
        // acoral_print("\r\n");
        // data_ready = 0;
    }
     else
    {
        Radio->StartRx();
        Radio->Process();
        Radio->Process();
        Radio->Process();    
    }

}

/**
 * @brief 从机服务函数slave，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void slave(void *args)
{
    if(data_ready)
    {
        Buffer[5] = data_ready;
        Radio->SetTxPacket( Buffer, sizeof(Buffer) ); 
        Radio->Process();
        Radio->Process();
        Radio->Process();
        acoral_print("Distance: %d cm\r\nTemp:%d.%d    Humi:%d.%d\r\nsignificant bit:%d", Buffer[0],Buffer[3],Buffer[4],Buffer[1],Buffer[2],Buffer[5]);
        acoral_print("\r\n");
        data_ready = 0;
    }
    else
    {
        Radio->StartRx();
        Radio->Process();
        Radio->Process();
        Radio->Process();    
    }
}

