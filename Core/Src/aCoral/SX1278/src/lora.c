/**
* @file: lora.c
* @author: 王若宇
* @brief: 主机从机服务函数的定义，简单实现主机不断发送测试数据，从机接收后串口打印数据
* @version: 1.0
* @date: 2023-05-13
*/
#include"lora.h"


tRadioDriver *Radio;                                    
uint8_t  Buffer[BUFFER_SIZE] = {0};                
uint8_t EnableMaster = true;
uint8_t master_data = 0; 




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


#if defined( MASTER )
    uint8_t master_device_id = 0x00;//高2位表示设备类型，00表示中心站，01表示终端，后6位表示设备地址
    uint8_t child_device[] = 
    {
        0x40,
        0x41,
        0x42,
        0x43,
        0x44,
        0x45,
        0x46,
        0x47,
        0x48,
        0x49,
        0x4A,
        0x4B,
        0x4C,
        0x4D,
        0x4E,
        0x4F,
        0x50,
        0x51,
        0x52,
        0x53,
    };
    uint8_t slave_Data[30] = {0};
   

/**
 * @brief 中心站发送服务函数master_tx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void master_tx(void *args)
{  
    if(data_4g)
    {      
        Radio->SetTxPacket(buf_4g, 6); //buf_4g[0]终端设备id，buf_4g[1]中心站节点id，buf_4g[2]数据项设置位(bit 每项数据一个bit位控制),buf_4g[3~5]三个传感器采集周期
        while((Radio->Process()) != RF_TX_DONE);
        acoral_print(buf_4g);
        memset(buf_4g,0,6);
        data_4g = 0;
    }
    

}

/**
 * @brief 中心站发送服务函数master_rx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void master_rx(void *args)
{
    uint8_t timeout = 30;
    Radio->StartRx();
    while(((Radio->Process()) != RF_RX_DONE)&&(timeout>0))
    {
        timeout--;
        HAL_Delay(100);
    }
    if(rx_done)
    {
        uint16_t Size = 30;
        Radio->GetRxPacket(slave_Data,( uint16_t* )&Size);
        if(slave_Data[0] == master_device_id)
        {
            master_data = 1;           
        }
        rx_done = 0;

    } 
  
    


}


uint8_t get_master_data()
{
    return master_data;
}

void test()
{
    acoral_print("test\r\n");
}

uint8_t get_master_id()
{
    return master_device_id;
}



#endif

#if defined(SLAVE)
    uint8_t master_id = 0x00;//终端所属中心站id
    uint8_t slave_device_id = 0x40;//高2位表示设备类型，00表示中心站，01表示终端，后6位表示设备地址
    uint8_t rx_cmd[6];//中心站命令接收缓冲区

/**
 * @brief 终端发送服务函数slave_tx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void slave_tx(void *args)
{
    if(data_ready)
    {   
        uint8_t timeout = 100;
        
        Buffer[0] = master_id;//中心站id
        Buffer[1] = slave_device_id;//终端设备id
        Buffer[2] = data_ready;//传感器数据有效位

        Radio->SetTxPacket( Buffer, sizeof(Buffer) ); 
        while((Radio->Process() != RF_TX_DONE)&&(timeout>0))
        {
            timeout--;
            HAL_Delay(1);
        }
        if(tx_done)
        {
            acoral_print("master_id:%d slave_device_id:%d \r\nsignificant bit:%d\r\nTemp:%d.%d    Humi:%d.%d\r\n", Buffer[0],Buffer[1],Buffer[2],Buffer[5],Buffer[6],Buffer[3],Buffer[4]);
            acoral_print("Distance: %d cm\r\n", (int)(*((float *)(&Buffer[7]))));
            acoral_print("Acceleration X-Axis: %d mg\r\n", (int)(*((float *)(&Buffer[11]))));
            acoral_print("Acceleration Y-Axis: %d mg\r\n", (int)(*((float *)(&Buffer[15]))));
            acoral_print("Acceleration Z-Axis: %d mg\r\n", (int)(*((float *)(&Buffer[19]))));
            acoral_print("\r\n");
            // memset(Buffer,0,sizeof(Buffer));
            tx_done = 0;
            data_ready = 0;
            return;
        }
    }
    else
    {
        return;
    }
    
}

/**
 * @brief 终端接收服务函数slave_rx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void slave_rx(void *args)
{  
    uint8_t timeout = 30;
    Radio->StartRx();

    while(((Radio->Process()) != RF_RX_DONE)&&(timeout>0))
    {
        timeout--;
        HAL_Delay(100);
    }
    if(rx_done)
        {
            acoral_enter_critical();
            Radio->GetRxPacket( rx_cmd, sizeof(rx_cmd) );
            if((rx_cmd[0] == slave_device_id)&&(rx_cmd[1] == master_id))
            {
                acoral_print("command:%d,%d,%d,%d\r\n",rx_cmd[0],rx_cmd[1],rx_cmd[2],rx_cmd[3]);
            }
      
            rx_done = 0;
            memset(rx_cmd,0,sizeof(rx_cmd));
            acoral_exit_critical();
        }
}


void test()
{
    acoral_print("test\r\n");
}
#endif
