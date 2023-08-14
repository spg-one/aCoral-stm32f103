#include "string.h"
#include "run_4g.h"
#include "lora.h"
 
#define DEMO_DTU_TEST_DATA                      "ALIENTEK ATK-IDM750C TEST"
#define DEMO_DTU_NETDATA_RX_BUF                 (1024)
#define DEMO_DTU_TCP_SERVER_DOMAIN_NAME         "cloud.alientek.com"
#define DEMO_DTU_TCP_SERVER_PORT_NUMBER         "59666"
#define DEMO_DTU_TCP_YUANZI_DEVICE_NUMEBER      "08454275304740178768"
#define DEMO_DTU_TCP_YUANZI_DEVICE_PASSWORD     "12345678"

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
    ret  = atk_idm750c_at_test();
    /* ATK-IDM750C 工作模式设置 */
    ret += atk_idm750c_query_workmode(ATK_IDM750C_WORKMODE_NET);
    /* ATK-IDM750C 第一路连接使能状态：ON */
    ret += atk_idm750c_link1en(ATK_IDM750C_LINK1EN_ON);
    /* ATK-IDM750C 第一路连接参数 */
    ret += atk_idm750c_link1(ATK_IDM750C_LINK1MODE_TCP,DEMO_DTU_TCP_SERVER_DOMAIN_NAME,DEMO_DTU_TCP_SERVER_PORT_NUMBER);
    /* ATK-IDM750C 第一路连接模式：LONG */
    ret += atk_idm750c_link1md(ATK_IDM750C_LINK1MD_LONG);
    /* ATK-IDM750C 第一路超时重连时间 */
    ret +=atk_idm750c_link1_timeout();
    /* ATK-IDM750C 第二路连接使能状态：OFF */
    ret += atk_idm750c_link2en(ATK_IDM750C_LINK2EN_OFF);
    /* ATK-IDM750C 第三路连接使能状态：OFF */
    ret += atk_idm750c_link3en(ATK_IDM750C_LINK3EN_OFF);
    /* ATK-IDM750C 第四路连接使能状态：OFF */
    ret += atk_idm750c_link4en(ATK_IDM750C_LINK4EN_OFF);
    /* ATK-IDM750C 启用原子云 */
    ret += atk_idm750c_set_yuanziyun(ATK_IDM750C_SET_YUANZIYUN_ON);
    /* ATK-IDM750C 原子云设备编号 */
    ret += atk_idm750c_device_num(DEMO_DTU_TCP_YUANZI_DEVICE_NUMEBER);
    /* ATK-IDM750C 原子云设备密码 */
    ret += atk_idm750c_device_password(DEMO_DTU_TCP_YUANZI_DEVICE_PASSWORD);
    /* ATK-IDM750C 开启心跳包 */
    ret += atk_idm750c_heartbeat_package(ATK_IDM750C_HRTEN_ON);
    /* ATK-IDM750C 心跳包数据 */
    ret += atk_idm750c_heartbeat_package_data();
    /* ATK-IDM750C 心跳包时间间隔 */
    ret += atk_idm750c_heartbeat_package_interval();
    /* ATK-IDM750C 注册包默认关闭 */
    ret += atk_idm750c_registration_package(ATK_IDM750C_SET_REGEN_OFF);
    /* ATK-IDM750C 注册包数据 */
    ret += atk_idm750c_registration_package_data();
    /* ATK-IDM750C 注册包发送方式 */
    ret += atk_idm750c_registration_package_send_method();
    /* ATK-IDM750C 注册包数据类型 */
    ret += atk_idm750c_registration_package_data_type();
    /* ATK-IDM750C 进入透传状态 */
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

/**
* @author: 王若宇
* @brief: 4g接发数据
* @version: 1.0
* @date: 2023-08-09
*/

void tx_4g(void)
{
    acoral_enter_critical();
    if(master_data)
    {   
        // atk_idm750c_uart_printf("Master id: %d Slave id: %d \r\nDistance: %d cm\r\nTemp:%d.%d    Humi:%d.%d\r\nsignificant bit:%d",slave_Data[0], slave_Data[1], slave_Data[2],slave_Data[5],slave_Data[6],slave_Data[3],slave_Data[4],slave_Data[7]);
        atk_idm750c_uart_printf("master_id:%d slave_device_id:%d \r\nsignificant bit:%d\r\nTemp:%d.%d    Humi:%d.%d period:%ds\r\n", 
                                slave_Data.master_id,
                                slave_Data.slave_device_id,
                                slave_Data.data_significant,
                                slave_Data.temp_int,
                                slave_Data.temp_dec,
                                slave_Data.humi_int,
                                slave_Data.humi_dec,
                                slave_Data.temp_humi_period);
        atk_idm750c_uart_printf("Distance: %d cm   period:%ds\r\n", (int)(slave_Data.distance),slave_Data.distance_period);
        atk_idm750c_uart_printf("Acceleration X-Axis: %d mg\r\n", (int)(slave_Data.acceleration_x));
        atk_idm750c_uart_printf("Acceleration Y-Axis: %d mg\r\n", (int)(slave_Data.acceleration_y));
        atk_idm750c_uart_printf("Acceleration Z-Axis: %d mg period:%ds\r\n", (int)(slave_Data.acceleration_z),slave_Data.acceleration_period);
        atk_idm750c_uart_printf("\r\n");
        master_data=0;
    }
    else if (data_ready)
    {
        Buffer.master_id = master_device_id;//中心站id
        Buffer.data_significant = data_ready;//传感器数据有效位
        atk_idm750c_uart_printf("master_id:%d significant bit:%d\r\nTemp:%d.%d    Humi:%d.%d period:%ds\r\n", 
                                Buffer.master_id,
                                Buffer.data_significant,
                                Buffer.temp_int,
                                Buffer.temp_dec,
                                Buffer.humi_int,
                                Buffer.humi_dec,
                                Buffer.temp_humi_period);
        atk_idm750c_uart_printf("Distance: %d cm   period:%ds\r\n", (int)(Buffer.distance),Buffer.distance_period);
        atk_idm750c_uart_printf("Acceleration X-Axis: %d mg\r\n", (int)(Buffer.acceleration_x));
        atk_idm750c_uart_printf("Acceleration Y-Axis: %d mg\r\n", (int)(Buffer.acceleration_y));
        atk_idm750c_uart_printf("Acceleration Z-Axis: %d mg period:%ds\r\n", (int)(Buffer.acceleration_z),Buffer.acceleration_period);
        atk_idm750c_uart_printf("\r\n");
        tx_done = 0;
        data_ready = 0;
    }
    acoral_exit_critical();
    
    
}

/**
* @author: 王若宇
* @brief: 4g接发数据
* @version: 1.0
* @date: 2023-08-09
*/
void rx_4g(void)
{
        buf_4g = atk_idm750c_uart_rx_get_frame();
        acoral_enter_critical();
        if (buf_4g != NULL)
        {
            data_4g = 1;
            rx_cmd.data_type = (uint8_t)0x02u;
            rx_cmd.master_id = buf_4g[0];
            rx_cmd.slave_device_id = buf_4g[1];
            rx_cmd.command_significant = buf_4g[2];
            rx_cmd.update_distance_period = buf_4g[3];
            rx_cmd.update_temp_humi_period = buf_4g[4];
            rx_cmd.update_acceleration_period = buf_4g[5];
            
            atk_idm750c_uart_rx_restart();
        }
        acoral_exit_critical();
}