/**
* @file: lora.c
* @author: 王若宇
* @brief: 主机从机服务函数的定义，简单实现主机不断发送测试数据，从机接收后串口打印数据
* @version: 1.0
* @date: 2023-05-13
*/
#include"lora.h"

tRadioDriver *Radio = NULL;
#define BUFFER_SIZE     30                          
uint16_t BufferSize = BUFFER_SIZE;            
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
    // acoral_print("this is master\r\n");
    memcpy(Buffer,MY_TEST_Msg,sizeof(MY_TEST_Msg));                 //将测试数据拷贝到Buffer中
    
     
    switch( Radio->Process() )
    {
        case RF_RX_TIMEOUT:                                         //Radio->Process( )返回接收超时后继续发送测试数据， Radio->Process()返回类型在radio.h的tRFProcessReturnCodes枚举中定义
            Radio->SetTxPacket( Buffer, sizeof(Buffer) );
            break;
        case RF_RX_DONE:
            Radio->GetRxPacket( Buffer, ( uint16_t* )&BufferSize ); //接收完成后将接收到的数据拷贝至Buffer中
            acoral_print(Buffer);
            break;
        case RF_TX_DONE:
            Radio->SetTxPacket( Buffer, sizeof(Buffer) );           //发送完成后继续发送
            break;
        default:
            break;
    }
	
	
}

/**
 * @brief 从机服务函数slave，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void slave(void *args)
{
    // acoral_print("this is slave\r\n");
                       
    switch( Radio->Process( ))
    {
        case RF_RX_DONE:
              
            Radio->GetRxPacket( Buffer, ( uint16_t* )&BufferSize ); //接收完成后将数据拷贝到Buffer中        
            acoral_print(Buffer);
            acoral_print("\r\n");
            memset(Buffer,0,sizeof(Buffer));                        //完成对接收数据的操作后清空Buffer缓冲区
            break;
        case RF_TX_DONE:
            Radio->StartRx( );                                      //发送完成后进入接收，但当前没有设置从机的发送
            break;
        default:
            break;
    }
	
	return 0;
}

