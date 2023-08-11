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
 * @brief       ��ʼ��4gģ��
 * @param       ��
 * @retval      ��
 */
void init_4g(void)
{
    uint8_t ret;
  

    
    /* ��ʼ��ATK-IDM750C */
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
    /* ATK-IDM750C ATָ����� */
    ret  = atk_idm750c_at_test();
    /* ATK-IDM750C ����ģʽ���� */
    ret += atk_idm750c_query_workmode(ATK_IDM750C_WORKMODE_NET);
    /* ATK-IDM750C ��һ·����ʹ��״̬��ON */
    ret += atk_idm750c_link1en(ATK_IDM750C_LINK1EN_ON);
    /* ATK-IDM750C ��һ·���Ӳ��� */
    ret += atk_idm750c_link1(ATK_IDM750C_LINK1MODE_TCP,DEMO_DTU_TCP_SERVER_DOMAIN_NAME,DEMO_DTU_TCP_SERVER_PORT_NUMBER);
    /* ATK-IDM750C ��һ·����ģʽ��LONG */
    ret += atk_idm750c_link1md(ATK_IDM750C_LINK1MD_LONG);
    /* ATK-IDM750C ��һ·��ʱ����ʱ�� */
    ret +=atk_idm750c_link1_timeout();
    /* ATK-IDM750C �ڶ�·����ʹ��״̬��OFF */
    ret += atk_idm750c_link2en(ATK_IDM750C_LINK2EN_OFF);
    /* ATK-IDM750C ����·����ʹ��״̬��OFF */
    ret += atk_idm750c_link3en(ATK_IDM750C_LINK3EN_OFF);
    /* ATK-IDM750C ����·����ʹ��״̬��OFF */
    ret += atk_idm750c_link4en(ATK_IDM750C_LINK4EN_OFF);
    /* ATK-IDM750C ����ԭ���� */
    ret += atk_idm750c_set_yuanziyun(ATK_IDM750C_SET_YUANZIYUN_ON);
    /* ATK-IDM750C ԭ�����豸��� */
    ret += atk_idm750c_device_num(DEMO_DTU_TCP_YUANZI_DEVICE_NUMEBER);
    /* ATK-IDM750C ԭ�����豸���� */
    ret += atk_idm750c_device_password(DEMO_DTU_TCP_YUANZI_DEVICE_PASSWORD);
    /* ATK-IDM750C ���������� */
    ret += atk_idm750c_heartbeat_package(ATK_IDM750C_HRTEN_ON);
    /* ATK-IDM750C ���������� */
    ret += atk_idm750c_heartbeat_package_data();
    /* ATK-IDM750C ������ʱ���� */
    ret += atk_idm750c_heartbeat_package_interval();
    /* ATK-IDM750C ע���Ĭ�Ϲر� */
    ret += atk_idm750c_registration_package(ATK_IDM750C_SET_REGEN_OFF);
    /* ATK-IDM750C ע������� */
    ret += atk_idm750c_registration_package_data();
    /* ATK-IDM750C ע������ͷ�ʽ */
    ret += atk_idm750c_registration_package_send_method();
    /* ATK-IDM750C ע����������� */
    ret += atk_idm750c_registration_package_data_type();
    /* ATK-IDM750C ����͸��״̬ */
    ret += atk_idm750c_enter_transfermode();
    
    if (ret != 0)
    {
        acoral_print("**************************************************************************\r\n");
        acoral_print("ATK-IDM750C Configuration Failed ...\r\n");
        acoral_print("�밴�����²�����м��:\r\n");
        acoral_print("1.ʹ�õ�����λ������������DTU�ܷ񵥶���������\r\n");
        acoral_print("2.���DTU���ڲ�����STM32ͨѶ�Ĵ��ڲ����Ƿ�һ��\r\n");
        acoral_print("3.���DTU��STM32���ڵĽ����Ƿ���ȷ\r\n");
        acoral_print("4.���DTU�����Ƿ�������DTU�Ƽ�ʹ��12V/1A��Դ���磬��Ҫʹ��USB��5V��ģ�鹩�磡��\r\n");
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
* @author: ������
* @brief: 4g�ӷ�����
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
* @author: ������
* @brief: 4g�ӷ�����
* @version: 1.0
* @date: 2023-08-09
*/
void rx_4g(void)
{
        buf_4g = atk_idm750c_uart_rx_get_frame();
        if (buf_4g != NULL)
        {
            data_4g = 1;
            //��ս��ջ�����
            atk_idm750c_uart_rx_restart();
        }
}