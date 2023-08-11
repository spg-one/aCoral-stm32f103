#include "string.h"
#include "run_4g.h"
#include "user.h"
 
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
    if(master_data)
    {   
        atk_idm750c_uart_printf("Master id: %d Slave id: %d \r\nDistance: %d cm\r\nTemp:%d.%d    Humi:%d.%d\r\nsignificant bit:%d",slave_Data[0], slave_Data[1], slave_Data[2],slave_Data[5],slave_Data[6],slave_Data[3],slave_Data[4],slave_Data[7]);
        memset(slave_Data,0,8);
        master_data = 0;
    }
    
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
        if (buf_4g != NULL)
        {
            data_4g = 1;
            //清空接收缓冲区
            atk_idm750c_uart_rx_restart();
        }
}