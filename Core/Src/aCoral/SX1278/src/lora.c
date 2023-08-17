/**
* @file: lora.c
* @author: 王若宇
* @brief: 主机从机服务函数的定义，简单实现主机不断发送测试数据，从机接收后串口打印数据
* @version: 1.0
* @date: 2023-05-13
*/
#include"lora.h"
#include "hal_thread.h"


tRadioDriver *Radio;                                    
data_buffer  Buffer = {0};//采集数据缓冲区
data_buffer rx_cmd = {0};//命令接收缓冲区                
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

/**
* @author: 王若宇
* @brief: 线程周期更改函数
* @version: 1.0
* @date: 2023-08-17
*/
void period_change()
{
    period_private_data_t * private_data;
    /*更改超声波线程周期*/
    if((cur_period.significant&0x01u))
    {
        acoral_enter_critical();
        ((period_private_data_t *)(cur_threads.distance_thread)->private_data)->time = (cur_period.distance)*1000;
        private_data=(cur_threads.distance_thread)->private_data;
        acoral_list_del(&(cur_threads.distance_thread->waiting));
        if(((cur_threads.distance_thread)->state&ACORAL_THREAD_STATE_SUSPEND))
        {
            (cur_threads.distance_thread)->stack=(unsigned int *)((char *)(cur_threads.distance_thread)->stack_buttom+(cur_threads.distance_thread)->stack_size-4);
			HAL_STACK_INIT(&(cur_threads.distance_thread)->stack,private_data->route,period_thread_exit,private_data->args);
            acoral_rdyqueue_add((cur_threads.distance_thread));
        }
        period_thread_delay((cur_threads.distance_thread),private_data->time);
        // period_thread_delay(acoral_cur_thread,((period_private_data_t *)acoral_cur_thread->private_data)->time);
        cur_period.significant &= (~0x01u);
        acoral_exit_critical();
    }

    /*更改温湿度线程周期*/
    if((cur_period.significant&(0x01u<<1)))
    {
        acoral_enter_critical();
        ((period_private_data_t *)(cur_threads.temp_humi_thread)->private_data)->time = (cur_period.temp_humi)*1000;
        private_data=(cur_threads.temp_humi_thread)->private_data;
        acoral_list_del(&(cur_threads.temp_humi_thread->waiting));
        if(((cur_threads.temp_humi_thread)->state&ACORAL_THREAD_STATE_SUSPEND))
        {
            (cur_threads.temp_humi_thread)->stack=(unsigned int *)((char *)(cur_threads.temp_humi_thread)->stack_buttom+(cur_threads.temp_humi_thread)->stack_size-4);
			HAL_STACK_INIT(&(cur_threads.temp_humi_thread)->stack,private_data->route,period_thread_exit,private_data->args);
            acoral_rdyqueue_add((cur_threads.temp_humi_thread));
        }
        period_thread_delay((cur_threads.temp_humi_thread),private_data->time);
        // period_thread_delay(acoral_cur_thread,((period_private_data_t *)acoral_cur_thread->private_data)->time);
        cur_period.significant &= (~(0x01u<<1));
        acoral_exit_critical();
    }

    /*更改加速度线程周期*/
    if((cur_period.significant&(0x01u<<2)))
    {
        acoral_enter_critical();
        ((period_private_data_t *)(cur_threads.acceleration_thread)->private_data)->time = (cur_period.acceleration)*1000;
        private_data=(cur_threads.acceleration_thread)->private_data;
        acoral_list_del(&(cur_threads.acceleration_thread->waiting));
        if(((cur_threads.acceleration_thread)->state&ACORAL_THREAD_STATE_SUSPEND))
        {
            (cur_threads.acceleration_thread)->stack=(unsigned int *)((char *)(cur_threads.acceleration_thread)->stack_buttom+(cur_threads.acceleration_thread)->stack_size-4);
			HAL_STACK_INIT(&(cur_threads.acceleration_thread)->stack,private_data->route,period_thread_exit,private_data->args);
            acoral_rdyqueue_add((cur_threads.acceleration_thread));
        }
        period_thread_delay((cur_threads.acceleration_thread),private_data->time);
        // period_thread_delay(acoral_cur_thread,((period_private_data_t *)acoral_cur_thread->private_data)->time);
        cur_period.significant &= (~(0x01u<<2));
        acoral_exit_critical();
    }

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
    data_buffer slave_Data = {0};
   

/**
 * @brief 中心站发送服务函数master_tx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void master_tx(void *args)
{  
    if(data_4g)
    {    
        /**
        * 更改周期命令包含于buf_4g，其中buf_4g[0]表示中心站id，buf_4g[1]表示终端id，buf_4g[2]表示有效位，buf_4g[2]从低到高  
        * 依次表示超声波、温湿度、加速度周期是否更改，1表示是，0表示否。
        * buf_4g[3]表示超声波周期，buf_4g[4]表示温湿度周期，buf_4g[5]表示加速度周期
        * 例如buf_4g[]={0x00,0x00,0x07,0x06,0x06,0x06}表示将中心站0x00的超声波温湿度加速度周期更改为6s
        */
    
        /*发送给中心站的命令*/  
        if((rx_cmd.master_id==master_device_id)&&(rx_cmd.slave_device_id==master_device_id))
        {
            cur_period.significant = rx_cmd.command_significant;
            cur_period.distance = rx_cmd.update_distance_period;
            cur_period.temp_humi = rx_cmd.update_temp_humi_period;
            cur_period.acceleration = rx_cmd.update_acceleration_period;
            data_4g = 0;
            period_change();
        }
        /*发送给终端的命令*/
        else if((rx_cmd.master_id==master_device_id))
        {
            acoral_enter_critical();
            Radio->SetTxPacket((uint8_t *)(&rx_cmd), sizeof(rx_cmd) ); //buf_4g[0]终端设备id，buf_4g[1]中心站节点id，buf_4g[2]数据项设置位(bit 每项数据一个bit位控制),buf_4g[3~5]三个传感器采集周期
            while((Radio->Process()) != RF_TX_DONE);
            data_4g = 0;
            acoral_exit_critical();
        }
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
    acoral_enter_critical();
    if(rx_done)
    {
        
        Radio->GetRxPacket((uint8_t *)(&slave_Data),sizeof(slave_Data));
        if(slave_Data.data_type == (uint8_t)0x01u)
        {
            if(slave_Data.master_id == master_device_id)
            {
                master_data = 1;           
            }
        }
        rx_done = 0;
        

    } 
    acoral_exit_critical();
    


}



void test()
{
    acoral_print("test\r\n");
}





#endif

#if defined(SLAVE)
    uint8_t master_id = 0x00;//终端所属中心站id
    uint8_t slave_device_id = 0x40;//高2位表示设备类型，00表示中心站，01表示终端，后6位表示设备地址
    

/**
 * @brief 终端发送服务函数slave_tx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void slave_tx(void *args)
{
    if(data_ready)
    {   
        Buffer.data_type = 0x01u;
        Buffer.master_id = master_id;//中心站id
        Buffer.slave_device_id = slave_device_id;//终端设备id
        Buffer.data_significant = data_ready;//传感器数据有效位

        Radio->SetTxPacket( (uint8_t *)(&Buffer), sizeof(Buffer) ); 
        while((Radio->Process() != RF_TX_DONE));
        if(tx_done)
        {
            acoral_print("master_id:%d slave_device_id:%d \r\nsignificant bit:%d\r\nTemp:%d.%d    Humi:%d.%d period:%ds\r\n", 
                                Buffer.master_id,
                                Buffer.slave_device_id,
                                Buffer.data_significant,
                                Buffer.temp_int,
                                Buffer.temp_dec,
                                Buffer.humi_int,
                                Buffer.humi_dec,
                                Buffer.temp_humi_period);
            acoral_print("Distance: %d cm   period:%ds\r\n", (int)(Buffer.distance),Buffer.distance_period);
            acoral_print("Acceleration X-Axis: %d mg\r\n", (int)(Buffer.acceleration_x));
            acoral_print("Acceleration Y-Axis: %d mg\r\n", (int)(Buffer.acceleration_y));
            acoral_print("Acceleration Z-Axis: %d mg period:%ds\r\n", (int)(Buffer.acceleration_z),Buffer.acceleration_period);
            acoral_print("\r\n");
            // memset(Buffer,0,sizeof(Buffer));
            tx_done = 0;
            data_ready = 0;
            
        }
    }


}

/**
 * @brief 终端接收服务函数slave_rx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void slave_rx(void *args)
{  
    uint8_t timeout = 80;
    Radio->StartRx();
    while(((Radio->Process()) != RF_RX_DONE)&&(timeout>0))
    {
        timeout--;
        HAL_Delay(100);
    }
    acoral_enter_critical();
    if(rx_done)
        {
            Radio->GetRxPacket( (uint8_t *)(&rx_cmd), sizeof(rx_cmd) );
            if(rx_cmd.data_type == (uint8_t)0x02u)
            {
                if((rx_cmd.master_id == master_id )&&(rx_cmd.slave_device_id == slave_device_id))
                {
                    acoral_print("receive command\r\n");
                    cur_period.significant = rx_cmd.command_significant;
                    cur_period.distance = rx_cmd.update_distance_period;
                    cur_period.temp_humi = rx_cmd.update_temp_humi_period;
                    cur_period.acceleration = rx_cmd.update_acceleration_period;
                }
        
                
            }
        
            rx_done = 0;
            
        }
    acoral_exit_critical();
}


void test()
{
    acoral_print("test\r\n");
}
#endif
