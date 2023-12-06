#include "string.h"
#include "run_4g.h"
#include "lora.h"

#if defined(MASTER)  


/* MQTT */
#define DEMO_DTU_TEST_DATA                      "ALIENTEK ATK-IDM750C TEST"
#define DEMO_DTU_NETDATA_RX_BUF                 (1024)

#define DEMO_DTU_TCP_SERVER_DOMAIN_NAME         "shenzhen.typologi.top"
#define DEMO_DTU_TCP_SERVER_PORT_NUMBER         "1883"

#define CLIENT_ID "alientek"
#define SUB_TOPIC "telemetry/response"
#define PUB_TOPIC "telemetry/request"

uint8_t *buf_4g = 0;
uint8_t data_4g = 0; 


/**
 * @brief       初始化4g模块
 * @param       无
 * @retval      无
 */
void init_4g(void)
{
    uint8_t ret;
  

    
    /* 初始化ATK-IDM750C */
    ret = atk_idm750c_init(115200);
    if (ret != 0)
    {
        acoral_print("ATK-IDM750C init failed!\r\n");
        while (1)
        {
            HAL_Delay(200);
        }
    }
    acoral_print("Wait for Cat1 DTU to start, wait 10s....\r\n");

    /* ATK-IDM750C AT指令测试 */
    ret = atk_idm750c_at_test();
    /*工作模式设置*/
    ret += atk_idm750c_query_workmode(ATK_IDM750C_WORKMODE_MQTT);
    /*服务器信息*/
    ret += atk_idm750c_server_address(DEMO_DTU_TCP_SERVER_DOMAIN_NAME,DEMO_DTU_TCP_SERVER_PORT_NUMBER);
    /*clientid 信息*/
    ret += atk_idm750c_mqtt_clientid(CLIENT_ID);
    /*参数信息*/
    ret += atk_idm750c_mqtt_para_information(ATK_IDM750C_MQTT_ONE_MODE2);
    /*订阅主题信息*/
    ret += atk_idm750c_mqtt_sub_title(SUB_TOPIC);
    /*发布主题信息*/
    ret += atk_idm750c_mqtt_pub_title(PUB_TOPIC);
    /*进入透传状态 */
    ret += atk_idm750c_enter_transfermode();
    
    if (ret != 0)
    {
        acoral_print("**************************************************************************\r\n");
        acoral_print("ATK-IDM750C Configuration Failed ...\r\n");
        acoral_print("请按照以下步骤进行检查:\r\n");
        acoral_print("1.使用电脑上位机配置软件检查DTU能否单独正常工作\r\n");
        acoral_print("2.检查DTU串口参数与STM32通讯的串口参数是否一致\r\n");
        acoral_print("3.检查DTU与STM32串口的接线是否正确\r\n");
        acoral_print("4.检查DTU供电是否正常，DTU推荐使用12V/1A电源供电，不要使用USB的5V给模块供电！！\r\n");
        acoral_print("**************************************************************************\r\n\r\n");
        
        while (1)
        {
            HAL_Delay(200);
        }
    }
    else
    {
        acoral_print("4g init done\r\n");
    }
    
}




/* 原子云 */
// #define DEMO_DTU_TEST_DATA                      "ALIENTEK ATK-IDM750C TEST"
// #define DEMO_DTU_NETDATA_RX_BUF                 (1024)
// #define DEMO_DTU_TCP_SERVER_DOMAIN_NAME         "cloud.alientek.com"
// #define DEMO_DTU_TCP_SERVER_PORT_NUMBER         "59666"
// #define DEMO_DTU_TCP_YUANZI_DEVICE_NUMEBER      "73190870637111929102"
// #define DEMO_DTU_TCP_YUANZI_DEVICE_PASSWORD     "12345678"

// uint8_t *buf_4g = 0;
// uint8_t data_4g = 0; 
// /**
//  * @brief       初始化4g模块
//  * @param       无
//  * @retval      无
//  */
// void init_4g(void)
// {
//     uint8_t ret;
  

    
//     /* 初始化ATK-IDM750C */
//     ret = atk_idm750c_init(115200);
//     if (ret != 0)
//     {
//         acoral_print("ATK-IDM750C init failed!\r\n");
//         while (1)
//         {
//             HAL_Delay(200);
//         }
//     }
//     acoral_print("Wait for Cat1 DTU to start, wait 10s....\r\n");
//     /* ATK-IDM750C AT指令测试 */
//     ret  = atk_idm750c_at_test();
//     /* ATK-IDM750C 工作模式设置 */
//     ret += atk_idm750c_query_workmode(ATK_IDM750C_WORKMODE_NET);
//     /* ATK-IDM750C 第一路连接使能状态：ON */
//     ret += atk_idm750c_link1en(ATK_IDM750C_LINK1EN_ON);
//     /* ATK-IDM750C 第一路连接参数 */
//     ret += atk_idm750c_link1(ATK_IDM750C_LINK1MODE_TCP,DEMO_DTU_TCP_SERVER_DOMAIN_NAME,DEMO_DTU_TCP_SERVER_PORT_NUMBER);
//     /* ATK-IDM750C 第一路连接模式：LONG */
//     ret += atk_idm750c_link1md(ATK_IDM750C_LINK1MD_LONG);
//     /* ATK-IDM750C 第一路超时重连时间 */
//     ret +=atk_idm750c_link1_timeout();
//     /* ATK-IDM750C 第二路连接使能状态：OFF */
//     ret += atk_idm750c_link2en(ATK_IDM750C_LINK2EN_OFF);
//     /* ATK-IDM750C 第三路连接使能状态：OFF */
//     ret += atk_idm750c_link3en(ATK_IDM750C_LINK3EN_OFF);
//     /* ATK-IDM750C 第四路连接使能状态：OFF */
//     ret += atk_idm750c_link4en(ATK_IDM750C_LINK4EN_OFF);
//     /* ATK-IDM750C 启用原子云 */
//     ret += atk_idm750c_set_yuanziyun(ATK_IDM750C_SET_YUANZIYUN_ON);
//     /* ATK-IDM750C 原子云设备编号 */
//     ret += atk_idm750c_device_num(DEMO_DTU_TCP_YUANZI_DEVICE_NUMEBER);
//     /* ATK-IDM750C 原子云设备密码 */
//     ret += atk_idm750c_device_password(DEMO_DTU_TCP_YUANZI_DEVICE_PASSWORD);
//     /* ATK-IDM750C 开启心跳包 */
//     ret += atk_idm750c_heartbeat_package(ATK_IDM750C_HRTEN_ON);
//     /* ATK-IDM750C 心跳包数据 */
//     ret += atk_idm750c_heartbeat_package_data();
//     /* ATK-IDM750C 心跳包时间间隔 */
//     ret += atk_idm750c_heartbeat_package_interval();
//     /* ATK-IDM750C 注册包默认关闭 */
//     ret += atk_idm750c_registration_package(ATK_IDM750C_SET_REGEN_OFF);
//     /* ATK-IDM750C 注册包数据 */
//     ret += atk_idm750c_registration_package_data();
//     /* ATK-IDM750C 注册包发送方式 */
//     ret += atk_idm750c_registration_package_send_method();
//     /* ATK-IDM750C 注册包数据类型 */
//     ret += atk_idm750c_registration_package_data_type();
//     /* ATK-IDM750C 进入透传状态 */
//     ret += atk_idm750c_enter_transfermode();
    
//     if (ret != 0)
//     {
//         acoral_print("**************************************************************************\r\n");
//         acoral_print("ATK-IDM750C Configuration Failed ...\r\n");
//         acoral_print("请按照以下步骤进行检查:\r\n");
//         acoral_print("1.使用电脑上位机配置软件检查DTU能否单独正常工作\r\n");
//         acoral_print("2.检查DTU串口参数与STM32通讯的串口参数是否一致\r\n");
//         acoral_print("3.检查DTU与STM32串口的接线是否正确\r\n");
//         acoral_print("4.检查DTU供电是否正常，DTU推荐使用12V/1A电源供电，不要使用USB的5V给模块供电！！\r\n");
//         acoral_print("**************************************************************************\r\n\r\n");
        
//         while (1)
//         {
//             HAL_Delay(200);
//         }
//     }
//     else
//     {
//         acoral_print("4g init done\r\n");
//     }
    
// }


/**
* @author: 贾苹
* @brief: 格式化4g发送数据
* @version: 2.0
* @date: 2023-09-08
*/


/**
* @author: 贾苹
* @brief: 4g发送数据
* @version: 2.0
* @date: 2023-09-08
*/
void tx_4g(void)
{
    // long level = acoral_enter_critical();
    if(master_data)//中心站从终端接收到的数据包校验和一致，就传输
    {   
        // acoral_print("slave:%d send data\r\n",slave_Data.slave_device_id);
        atk_idm750c_uart_printf("{\"masterId\":%d,\"slaveDeviceId\":%d,\"dataSignificant\":%d,",slave_Data.master_id,slave_Data.slave_device_id,slave_Data.data_significant);
        atk_idm750c_uart_printf("\"temp\":%d.%d,\"humi\":%d.%d,\"distance\":%d,\"accelerationX\":%d,\"accelerationY\":%d,\"accelerationZ\":%d,",
                                slave_Data.temp_int,
                                slave_Data.temp_dec,
                                slave_Data.humi_int,
                                slave_Data.humi_dec,
                                (int)slave_Data.distance,
                                (int)slave_Data.acceleration_x,
                                (int)slave_Data.acceleration_y,
                                (int)slave_Data.acceleration_z);
        atk_idm750c_uart_printf("\"tempHumiPeriod\":%d,\"distancePeriod\":%d,\"accelerationPeriod\":%d,",
                                slave_Data.temp_humi_period,slave_Data.distance_period,slave_Data.acceleration_period);
        atk_idm750c_uart_printf("\"tempCollectTime\":%d,",slave_Data.temp_collect_time);
        atk_idm750c_uart_printf("\"distanceCollectTime\":%d,",slave_Data.distance_collect_time);
        atk_idm750c_uart_printf("\"accelerationCollectTime\":%d}",slave_Data.acceleration_collect_time);
        master_data=0;
    }
    else if (data_ready)
    {
        // acoral_print("4G master send data\r\n");
        Buffer.master_id = master_device_id;//中心站id
        Buffer.slave_device_id = master_device_id;
        Buffer.data_significant = data_ready;//传感器数据有效位
        atk_idm750c_uart_printf("{\"masterId\":%d,\"slaveDeviceId\":%d,\"dataSignificant\":%d,",Buffer.master_id,Buffer.master_id,Buffer.data_significant);
        atk_idm750c_uart_printf("\"temp\":%d.%d,\"humi\":%d.%d,\"distance\":%d,\"accelerationX\":%d,\"accelerationY\":%d,\"accelerationZ\":%d,",
                                Buffer.temp_int,
                                Buffer.temp_dec,
                                Buffer.humi_int,
                                Buffer.humi_dec,
                                (int)Buffer.distance,
                                (int)Buffer.acceleration_x,
                                (int)Buffer.acceleration_y,
                                (int)Buffer.acceleration_z);
        atk_idm750c_uart_printf("\"tempHumiPeriod\":%d,\"distancePeriod\":%d,\"accelerationPeriod\":%d,",
                                Buffer.temp_humi_period,Buffer.distance_period,Buffer.acceleration_period);
        atk_idm750c_uart_printf("\"tempCollectTime\":%d,",Buffer.temp_collect_time);
        atk_idm750c_uart_printf("\"distanceCollectTime\":%d,",Buffer.distance_collect_time);
        atk_idm750c_uart_printf("\"accelerationCollectTime\":%d}",Buffer.acceleration_collect_time);

        // acoral_print("distance :%d  ,x: %d\t,y: %d  ,z: %d  \r\n",(int)Buffer.distance,(int)Buffer.acceleration_x,(int)Buffer.acceleration_y,(int)Buffer.acceleration_z);

        tx_done = 0; //lora每次发送完会自动置1，需要手动清零。
        memset(&Buffer,0,sizeof(Buffer)); 
        data_ready = 0;
        intr_flag = 0;
    }
    // acoral_exit_critical(level);
    
    
}


/**
* @author: 贾苹
* @brief: 中心站同步Syn_Tick的任务
* @version: 2.0
* @date: 2023-09-09
*/


/**
* @author: 贾苹
* @brief: 4g接发数据
* @version: 2.0
* @date: 2023-09-06
*/
void rx_4g(void)
{
    //05 00 40 07 1F 1F 1F 命令类型 中心节点id 终端节点id 更改周期位数 更改的周期x3
    //0500FF00000000 同步
    buf_4g = atk_idm750c_uart_rx_get_frame();
    // long level = acoral_enter_critical();
    if (buf_4g != NULL)
    {
        // acoral_print("4G receving....rx_done:%d-%d-%d\r\n",buf_4g[0],buf_4g[1],buf_4g[2]);
        if(buf_4g[1] == master_device_id)
        {
            //发给中心站，表示不需要下发给终端
            if(buf_4g[2] == master_device_id)
            {
                if (buf_4g[3] != 0)
                {
                    cur_period.period_significant = buf_4g[3];
                    cur_period.distance = buf_4g[4];
                    cur_period.temp_humi = buf_4g[5];
                    cur_period.acceleration = buf_4g[6];
                    data_4g = 0;
                    period_change();
                }
            }
            else //发给终端的，FF表示发给所有终端(实际上也是接收到对应终端的数据包后单个发送给终端)，其余表示发给单个终端，服务器发送同步命令需要指定终端id
            {
                acoral_print("rec 4g data!\r\n");
                rx_cmd.master_id = buf_4g[1];
                rx_cmd.slave_device_id = buf_4g[2];
                rx_cmd.change_period_significant = buf_4g[3];
                rx_cmd.update_distance_period = buf_4g[4];
                rx_cmd.update_temp_humi_period = buf_4g[5];
                rx_cmd.update_acceleration_period = buf_4g[6];
                data_4g = 1;           // 标识收到了4G数据，并且需要发送
                if (buf_4g[0] == 0x05) // 接收到同步命令，将本地sync_tick清零，同时sync_flag置1，标识同步中
                {
                    sync_tick = 0;
                    sync_flag = 1;
                    
                    //唤醒中心站同步任务线程
                    acoral_thread_t *master_sync_thread;
                    master_sync_thread = (acoral_thread_t *)acoral_get_res_by_id(master_sync_thread_id);
                    acoral_rdy_thread(master_sync_thread);
                }
                
            }    
            
        }
        
    }
    atk_idm750c_uart_rx_restart();
    // acoral_exit_critical(level);
}

#endif