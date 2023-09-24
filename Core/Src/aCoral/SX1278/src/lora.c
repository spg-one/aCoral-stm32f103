/**
* @file: lora.c
* @author: 贾苹
* @brief: 主机从机服务函数的定义，简单实现主机不断发送测试数据，从机接收后串口打印数据
* @version: 2.0
* @date: 2023-09-07
*/
#include "lora.h"
#include "hal_thread.h"


tRadioDriver *Radio;                                    
data_buffer  Buffer = {0};  //采集数据缓冲区
data_buffer rx_cmd = {0};   //命令接收缓冲区                
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
* @author: 贾苹
* @brief: 线程周期更改函数
* @version: 2.0
* @date: 2023-08-25
*/
void period_change()
{
    period_private_data_t * private_data;
    /*更改超声波线程周期*/
    if((cur_period.significant&0x01u))
    {
        acoral_enter_critical();
        acoral_list_del(&(cur_threads.distance_thread->waiting));//从周期延时队列中删去
        ((period_private_data_t *)(cur_threads.distance_thread)->private_data)->time = (cur_period.distance)*1000; //更改当前线程的私有数据周期
        private_data=(cur_threads.distance_thread)->private_data;

        //创建一个新线程
        (cur_threads.distance_thread)->stack = (unsigned int *)((char *)(cur_threads.distance_thread)->stack_buttom + (cur_threads.distance_thread)->stack_size - 4);
        HAL_STACK_INIT(&(cur_threads.distance_thread)->stack, private_data->route, period_thread_exit, private_data->args);
        acoral_rdyqueue_add((cur_threads.distance_thread));

        //将新线程挂到周期延时队列
        period_thread_delay((cur_threads.distance_thread),private_data->time); //更新period_delay_queue队列上的线程的time
        cur_period.significant &= (~0x01u);
        acoral_exit_critical();
    }

    /*更改温湿度线程周期*/
    if((cur_period.significant&(0x01u<<1)))
    {
        acoral_enter_critical();
        acoral_list_del(&(cur_threads.temp_humi_thread->waiting)); 
        ((period_private_data_t *)(cur_threads.temp_humi_thread)->private_data)->time = (cur_period.temp_humi)*1000;
        private_data=(cur_threads.temp_humi_thread)->private_data;

        (cur_threads.temp_humi_thread)->stack = (unsigned int *)((char *)(cur_threads.temp_humi_thread)->stack_buttom + (cur_threads.temp_humi_thread)->stack_size - 4);
        HAL_STACK_INIT(&(cur_threads.temp_humi_thread)->stack, private_data->route, period_thread_exit, private_data->args);
        acoral_rdyqueue_add((cur_threads.temp_humi_thread));

        period_thread_delay((cur_threads.temp_humi_thread),private_data->time);
        // period_thread_delay(acoral_cur_thread,((period_private_data_t *)acoral_cur_thread->private_data)->time);
        cur_period.significant &= (~(0x01u<<1));
        acoral_exit_critical();
    }

    /*更改加速度线程周期*/
    if((cur_period.significant&(0x01u<<2)))
    {
        acoral_enter_critical();
        acoral_list_del(&(cur_threads.acceleration_thread->waiting));
        ((period_private_data_t *)(cur_threads.acceleration_thread)->private_data)->time = (cur_period.acceleration)*1000;
        private_data=(cur_threads.acceleration_thread)->private_data;

        (cur_threads.acceleration_thread)->stack = (unsigned int *)((char *)(cur_threads.acceleration_thread)->stack_buttom + (cur_threads.acceleration_thread)->stack_size - 4);
        HAL_STACK_INIT(&(cur_threads.acceleration_thread)->stack, private_data->route, period_thread_exit, private_data->args);
        acoral_rdyqueue_add((cur_threads.acceleration_thread));

        period_thread_delay((cur_threads.acceleration_thread),private_data->time);
        // period_thread_delay(acoral_cur_thread,((period_private_data_t *)acoral_cur_thread->private_data)->time);
        cur_period.significant &= (~(0x01u<<2));
        acoral_exit_critical();
    }

}

uint8_t compute_check_value(data_buffer  Buffer)
{
    uint8_t check_value = 0;
    check_value += (uint8_t)Buffer.distance;
    check_value += Buffer.humi_int;
    check_value += Buffer.humi_dec;
    check_value += Buffer.temp_int;
    check_value += Buffer.temp_dec;
    check_value += (uint8_t)Buffer.acceleration_x;
    check_value += (uint8_t)Buffer.acceleration_y;
    check_value += (uint8_t)Buffer.acceleration_z;
    return check_value;
}

#if defined( MASTER )
    uint8_t master_device_id = 0x00;//00000001 高2位表示设备类型，00表示中心站，01表示终端，后6位表示设备地址
    uint8_t child_device[] = //01000000
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
 * @return 1：校验和一致可以发送数据到服务器，0表示有错误。
 */
uint8_t master_tx()
{   
    //收到了需要下发的4G命令，更改周期+同步
    if(data_4g)
    {    
        /**
        * 有变化，待重写?
        * 更改周期命令包含于buf_4g，其中buf_4g[0]表示中心站id，buf_4g[1]表示终端id，buf_4g[2]表示有效位，buf_4g[2]从低到高  
        * 依次表示超声波、温湿度、加速度周期是否更改，1表示是，0表示否。
        * buf_4g[3]表示超声波周期，buf_4g[4]表示温湿度周期，buf_4g[5]表示加速度周期
        * 例如buf_4g[]={0x00,0x00,0x07,0x06,0x06,0x06}表示将中心站0x00的超声波温湿度加速度周期更改为6s
        */
        acoral_print("4G tx\r\n");
        //如果是群发，需要等待接收数据包后再依次发送
        if (rx_cmd.slave_device_id == (uint8_t)0x7fu)
        {
            rx_cmd.slave_device_id = slave_Data.slave_device_id;
        }
        
        // 若接收到的校验值与计算出来的校验值一致
        if (slave_Data.check_value == compute_check_value(slave_Data))
        {
            rx_cmd.data_type = (uint8_t)0x02u;
            acoral_enter_critical();
            // 正在同步中，就需要判断终端是否接收到syn_tick值，或者终端的syn_tick与中心站之差是否在阈值范围内，如果不是则需要重新传递syn_tick
            if (sync_flag == 1 && ((sync_tick - slave_Data.sync_tick > tick_diff_threshold) || (slave_Data.sync_tick == 0))) // 需调试？
            {
                acoral_print("First local Tick:%d\trecv Tick:%d\tdiff:%d\r\n",sync_tick,slave_Data.sync_tick,sync_tick - slave_Data.sync_tick);
                rx_cmd.sync_tick = sync_tick;
            }
            Radio->SetTxPacket((uint8_t *)(&rx_cmd), sizeof(rx_cmd)); // buf_4g[0]终端设备id，buf_4g[1]中心站节点id，buf_4g[2]数据项设置位(bit 每项数据一个bit位控制),buf_4g[3~5]三个传感器采集周期
            acoral_exit_critical();
            while ((Radio->Process()) != RF_TX_DONE);
            data_4g = 0;
            memset(&rx_cmd,0,sizeof(rx_cmd)); 
            return 1;
        }
        else
        {
            rx_cmd.data_type = (uint8_t)0x03u;
            acoral_enter_critical();
            if (sync_flag == 1 && ((sync_tick - slave_Data.sync_tick > tick_diff_threshold) || (slave_Data.sync_tick == 0)))
            {
                rx_cmd.sync_tick = sync_tick;
            }
            Radio->SetTxPacket((uint8_t *)(&rx_cmd), sizeof(rx_cmd)); // buf_4g[0]终端设备id，buf_4g[1]中心站节点id，buf_4g[2]数据项设置位(bit 每项数据一个bit位控制),buf_4g[3~5]三个传感器采集周期
            acoral_exit_critical();
            while ((Radio->Process()) != RF_TX_DONE);
            data_4g = 0;
        }

        data_4g = 0;
    }
    else{
        //未收到4G命令，不需要更改周期
        //rx_cmd.change_period_significant = 0;
        rx_cmd.master_id = slave_Data.master_id;//中心站id
        rx_cmd.slave_device_id = slave_Data.slave_device_id;//终端设备id
        //若接收到的校验值与计算出来的校验值一致
        if (slave_Data.check_value == compute_check_value(slave_Data))
        {
            rx_cmd.data_type = (uint8_t)0x02u;
            acoral_enter_critical();
            if(sync_flag == 1 && ((sync_tick-slave_Data.sync_tick>tick_diff_threshold) || (slave_Data.sync_tick==0)))
            {
                rx_cmd.sync_tick = sync_tick;
                acoral_print("local Tick:%d\trecv Tick:%d\tdiff:%d\r\n",sync_tick,slave_Data.sync_tick,sync_tick - slave_Data.sync_tick);
                acoral_print("-----------------resend Tick-----------------\r\n");
            }else if (sync_flag == 1)
            {
                acoral_print("local Tick:%d\trecv Tick:%d\tdiff:%d\r\n",sync_tick,slave_Data.sync_tick,sync_tick - slave_Data.sync_tick);
                acoral_print("-----------------Tick similar----------------\r\n");
            }
            Radio->SetTxPacket((uint8_t *)(&rx_cmd), sizeof(rx_cmd) ); //buf_4g[0]终端设备id，buf_4g[1]中心站节点id，buf_4g[2]数据项设置位(bit 每项数据一个bit位控制),buf_4g[3~5]三个传感器采集周期
            acoral_exit_critical();
            while((Radio->Process()) != RF_TX_DONE);
            memset(&rx_cmd,0,sizeof(rx_cmd)); 
            return 1;
        }
        else{
            rx_cmd.data_type = (uint8_t)0x03u;
            acoral_enter_critical();
            if(sync_flag == 1 && ((sync_tick-slave_Data.sync_tick>tick_diff_threshold) || (slave_Data.sync_tick==0)))
            {
                rx_cmd.sync_tick = sync_tick;
            }
            Radio->SetTxPacket((uint8_t *)(&rx_cmd), sizeof(rx_cmd) ); //buf_4g[0]终端设备id，buf_4g[1]中心站节点id，buf_4g[2]数据项设置位(bit 每项数据一个bit位控制),buf_4g[3~5]三个传感器采集周期
            acoral_exit_critical();
            while((Radio->Process()) != RF_TX_DONE);
        }
    }
    memset(&rx_cmd,0,sizeof(rx_cmd)); 
    return 0;
}

/**
 * @brief 中心站发送服务函数master_rx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void master_rx(void *args)
{
    uint8_t timeout = 20;
    Radio->StartRx();
    while(((Radio->Process()) != RF_RX_DONE)&&(timeout>0))
    {
        timeout--;
        HAL_Delay(100);
    }
    // acoral_enter_critical();
    if(sync_flag == 1)
    {
        acoral_print("local_tick:%d    ",sync_tick);
    }
    acoral_print("master receving....rx_done:%d\r\n",rx_done);
    if(rx_done)
    {
        Radio->GetRxPacket((uint8_t *)(&slave_Data),sizeof(slave_Data));
        if(slave_Data.data_type == 0x01u) //终端发来数据包
        {
            if(slave_Data.master_id == master_device_id)//判断是否是自己接收
            {
                //发送应答信号or应答信号+命令
                if(master_tx() == 1)
                {
                    master_data = 1;    
                }
                //若收到的数据有误，就先不发送给服务器   
            }
        }
        rx_done = 0;

    } 
    // acoral_exit_critical();
    


}
#endif

#if defined(SLAVE)
    uint8_t master_id = 0x00;      //终端所属中心站id
    // uint8_t slave_device_id = 0x40;//高2位表示设备类型，00表示中心站，01表示终端，后6位表示设备地址0100 0000
    uint8_t slave_device_id = 0x41;

/**
* @author: 贾苹
* @brief: 终端发送任务
* @param: task 0：普通任务调用此函数；1：时钟同步任务调用此函数
* @version: 2.0
* @date: 2023-09-09
*/
void send_data()
{
    Buffer.data_type = (uint8_t)0x01u;
    Buffer.master_id = master_id;             // 中心站id
    Buffer.slave_device_id = slave_device_id; // 终端设备id
    //若被打断，赋值可能会被改变
    acoral_enter_critical();
    Buffer.data_significant = data_ready; // 传感器数据有效位
    Buffer.check_value = compute_check_value(Buffer);
    if (sync_flag == 1)
    {
        Buffer.sync_tick = sync_tick;
    }
    Radio->SetTxPacket((uint8_t *)(&Buffer), sizeof(Buffer));
    acoral_exit_critical();
    
    while((Radio->Process() != RF_TX_DONE)); 
    //发送成功
    if (tx_done)
    {
        acoral_print("master_id:%d\tslave_device_id:%d \r\nsignificant bit:%d\r\nTemp:%d.%d\tHumi:%d.%d\tperiod:%ds\t",
                     Buffer.master_id,
                     Buffer.slave_device_id,
                     Buffer.data_significant,
                     Buffer.temp_int,
                     Buffer.temp_dec,
                     Buffer.humi_int,
                     Buffer.humi_dec,
                     Buffer.temp_humi_period);
        acoral_print("%d\r\n",Buffer.temp_collect_time);
        acoral_print("Distance:%dcm\t\t\tperiod:%ds\t", (int)(Buffer.distance), Buffer.distance_period);
        acoral_print("%d\r\n",Buffer.distance_collect_time);
        acoral_print("Acceleration X-Axis: %d mg\r\n", (int)(Buffer.acceleration_x));
        acoral_print("Acceleration Y-Axis: %d mg\r\n", (int)(Buffer.acceleration_y));
        acoral_print("Acceleration Z-Axis: %d mg\tperiod:%ds\t", (int)(Buffer.acceleration_z), Buffer.acceleration_period);
        acoral_print("%d\r\n",Buffer.acceleration_collect_time);
        acoral_print("Sync Tick: %d\r\n", Buffer.sync_tick);
        acoral_print("Check Value: %d\r\n\r\n", Buffer.check_value);
        
        tx_done = 0;

        // 开始等待中心站应答信号，将LoRa切换为接收模式，若校验值不一致则重传数据包三次，三次之后放弃此次发送。
        for (int i = 0; i < 2; i++)
        {
            if (slave_rx() == 1)
            {
                break;
            }
        }
        memset(&Buffer,0,sizeof(Buffer)); 
    }
}

/**
 * @brief 终端发送服务函数slave_tx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0 
 */
void slave_tx(void *args)
{
    if(data_ready) //一个传感器数据准备好了就可以发送
    {   
        send_data();
        data_ready = 0;
    }

}




/**
 * @brief 终端接收服务函数slave_rx，用户可根据需求更改接发逻辑
 * @param None  
 * @return 0，应答数据包不一致重新发送数据；1，应答数据包一致更改周期参数
 */
uint8_t slave_rx()
{  
    uint8_t timeout = 10;
    Radio->StartRx();
    while(((Radio->Process()) != RF_RX_DONE)&&(timeout>0))  //没有接收完数据，并且没有超时
    {
        timeout--;
        HAL_Delay(100);
    }

    //acoral_enter_critical(); 已经是最高优先级5任务，无需进入临界区
    acoral_print("slave receving....rx_done:%d\r\n\r\n",rx_done);
    if(rx_done)
    {
        Radio->GetRxPacket((uint8_t *)(&rx_cmd), sizeof(rx_cmd));
        rx_done = 0;
        // 判断是否是发送给本终端的应答包
        if ((rx_cmd.master_id == master_id) && (rx_cmd.slave_device_id == slave_device_id)) 
        {

            //同步任务
            if (rx_cmd.sync_tick != 0)
            {
                if(sync_flag == 0) //第一次收到sync_tick
                {
                    sync_tick = rx_cmd.sync_tick;
                    sync_flag = 1;
                    // 唤醒终端同步任务线程，优先级与通信线程优先级一致
                    acoral_thread_t *slave_sync_thread;
                    slave_sync_thread = (acoral_thread_t *)acoral_get_res_by_id(slave_sync_thread_id);
                    acoral_rdy_thread(slave_sync_thread);

                }
                else if (sync_flag == 1)
                {   
                    sync_tick = rx_cmd.sync_tick;
                }
            }
            else //收到的sync_tick为0
            {
                if(sync_flag == 1)
                {
                    sync_flag = 2;//syn_tick同步成功
                }
                //syn_tick为0且syn_flag为0，表示未开启同步
            }
            
            // 更改周期
            if (rx_cmd.change_period_significant != 0)
            {
                acoral_print("receive command\r\n");
                cur_period.significant = rx_cmd.change_period_significant;
                cur_period.distance = rx_cmd.update_distance_period;
                cur_period.temp_humi = rx_cmd.update_temp_humi_period;
                cur_period.acceleration = rx_cmd.update_acceleration_period;
                period_change();
            }
            // 检验上次数据包是否发送成功
            if (rx_cmd.data_type == (uint8_t)0x02u)
            {
                acoral_print("check right \r\n");
                return 1;
            }
            else if (rx_cmd.data_type == (uint8_t)0x03u)
            {
                acoral_print("check wrong \r\n");
                // 若应答信号不一致，则重新发送原有数据包
                acoral_enter_critical();
                Buffer.data_significant = data_ready;//传感器数据有效位
                Buffer.check_value = compute_check_value(Buffer);
                if (sync_flag == 1)
                {
                    Buffer.sync_tick = sync_tick;
                }
                Radio->SetTxPacket((uint8_t *)(&Buffer), sizeof(Buffer));
                acoral_exit_critical();
                while ((Radio->Process() != RF_TX_DONE));
                if (tx_done)
                {
                    acoral_print("Data Wrong,Resend data!!!!\r\n");
                    acoral_print("master_id:%d\tslave_device_id:%d \r\nsignificant bit:%d\r\nTemp:%d.%d\tHumi:%d.%d\tperiod:%ds\t",
                                 Buffer.master_id,
                                 Buffer.slave_device_id,
                                 Buffer.data_significant,
                                 Buffer.temp_int,
                                 Buffer.temp_dec,
                                 Buffer.humi_int,
                                 Buffer.humi_dec,
                                 Buffer.temp_humi_period);
                    acoral_print("%d\r\n", Buffer.temp_collect_time);
                    acoral_print("Distance:%dcm\t\t\tperiod:%ds\t", (int)(Buffer.distance), Buffer.distance_period);
                    acoral_print("%d\r\n", Buffer.distance_collect_time);
                    acoral_print("Acceleration X-Axis: %d mg\r\n", (int)(Buffer.acceleration_x));
                    acoral_print("Acceleration Y-Axis: %d mg\r\n", (int)(Buffer.acceleration_y));
                    acoral_print("Acceleration Z-Axis: %d mg\tperiod:%ds\t", (int)(Buffer.acceleration_z), Buffer.acceleration_period);
                    acoral_print("%d\r\n",Buffer.acceleration_collect_time);
                    acoral_print("Sync Tick: %d\r\n", Buffer.sync_tick);
                    acoral_print("Check Value: %d\r\n\r\n", Buffer.check_value);
                    tx_done = 0;
                }
            }
        }
    }
    else
    {
        //没有接收到数据，也应该重发
        acoral_enter_critical();
        Buffer.data_significant = data_ready;//传感器数据有效位
        Buffer.check_value = compute_check_value(Buffer);
        if (sync_flag == 1)
        {
            Buffer.sync_tick = sync_tick;
        }
        Radio->SetTxPacket((uint8_t *)(&Buffer), sizeof(Buffer));
        acoral_exit_critical();
        while ((Radio->Process() != RF_TX_DONE));
        if (tx_done)
        {
            acoral_print("No Response,Resend data!!!!!!!!\r\n");
            acoral_print("master_id:%d\tslave_device_id:%d \r\nsignificant bit:%d\r\nTemp:%d.%d\tHumi:%d.%d\tperiod:%ds\t",
                         Buffer.master_id,
                         Buffer.slave_device_id,
                         Buffer.data_significant,
                         Buffer.temp_int,
                         Buffer.temp_dec,
                         Buffer.humi_int,
                         Buffer.humi_dec,
                         Buffer.temp_humi_period);
            acoral_print("%d\r\n", Buffer.temp_collect_time);
            acoral_print("Distance:%dcm\t\t\tperiod:%ds\t", (int)(Buffer.distance), Buffer.distance_period);
            acoral_print("%d\r\n", Buffer.distance_collect_time);
            acoral_print("Acceleration X-Axis: %d mg\r\n", (int)(Buffer.acceleration_x));
            acoral_print("Acceleration Y-Axis: %d mg\r\n", (int)(Buffer.acceleration_y));
            acoral_print("Acceleration Z-Axis: %d mg\tperiod:%ds\t", (int)(Buffer.acceleration_z), Buffer.acceleration_period);
            acoral_print("%d\r\n", Buffer.acceleration_collect_time);
            acoral_print("Sync Tick: %d\r\n", Buffer.sync_tick);
            acoral_print("Check Value: %d\r\n\r\n", Buffer.check_value);
            tx_done = 0;
        }
    }
    //acoral_exit_critical();
    return 0;
}

#endif

