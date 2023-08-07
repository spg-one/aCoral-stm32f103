#include "atk_idm750c_cfg.h"
#include "atk_idm750c_uart.h"
#include "atk_idm750c.h"
#include "string.h"

static uint8_t dtu_rxcmdbuf[DTU_RX_CMD_BUF_SIZE];       /*����DTU������ݻ���*/

/**
 * @brief       ATK-IDM750Cģ���ʼ��
 * @param       baudrate: ATK-IDM750C UARTͨѶ������
 * @retval      ATK_IDM750C_EOK  : ATK-IDM750C��ʼ���ɹ�������ִ�гɹ�
 *              ATK_IDM750C_ERROR: ATK-IDM750C��ʼ��ʧ�ܣ�����ִ��ʧ��
 */
uint8_t atk_idm750c_init(uint32_t baudrate)
{
    atk_idm750c_uart_init(baudrate);
    atk_idm750c_dtu_enter_configmode();
    if (atk_idm750c_at_test() != ATK_IDM750C_EOK)   /* ATK-IDM750C ATָ����� */
    {
        return ATK_IDM750C_ERROR;
    }
    
    return ATK_IDM750C_EOK;
}

/**
 * @brief       DTU�Զ��ϱ�URC��Ϣ������:����+ATK ERROR��Ϣ
 * @param       data    :   ���յ�DTU��URC���ݻ���
 * @param       len     :   URC���ݳ���
 * @return      ��
 */
static void dtu_urc_atk_error(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_atk_error\r\n");
}

/**
 * @brief       DTU�Զ��ϱ�URC��Ϣ������:����Please check SIM Card��Ϣ
 * @param       data    :   ���յ�DTU��URC���ݻ���
 * @param       len     :   URC���ݳ���
 * @return      ��
 */
static void dtu_urc_error_sim(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_error_sim\r\n");
}

/**
 * @brief       DTU�Զ��ϱ�URC��Ϣ������:����Please check GPRS��Ϣ
 * @param       data    :   ���յ�DTU��URC���ݻ���
 * @param       len     :   URC���ݳ���
 * @return      ��
 */
static void dtu_urc_error_gprs(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_error_gprs\r\n");
}

/**
 * @brief       DTU�Զ��ϱ�URC��Ϣ������:����Please check CSQ��Ϣ
 * @param       data    :   ���յ�DTU��URC���ݻ���
 * @param       len     :   URC���ݳ���
 * @return      ��
 */
static void dtu_urc_error_csq(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_error_csq\r\n");
}

/**
 * @brief       DTU�Զ��ϱ�URC��Ϣ������:����Please check MQTT Parameter��Ϣ
 * @param       data    :   ���յ�DTU��URC���ݻ���
 * @param       len     :   URC���ݳ���
 * @return      ��
 */
static void dtu_urc_error_mqtt(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_error_mqtt\r\n");
}

static _dtu_urc_st DTU_ATK_M750_URC[ATK_IDM750C_DTU_URC_SIZE] =
{
    {"+ATK ERROR:",                         dtu_urc_atk_error},         /*DTU�������⣬��Ҫ��ϵ����֧�ֽ���ȷ��*/
    {"Please check SIM Card !!!\r\n",       dtu_urc_error_sim},         /*DTUδ��⵽�ֻ���,�����ֻ����Ƿ���ȷ����*/
    {"Please check GPRS !!!\r\n",           dtu_urc_error_gprs},        /*����SIM���Ƿ�Ƿ��*/
    {"Please check CSQ !!!\r\n",            dtu_urc_error_csq},         /*���������Ƿ���ȷ���룬��ȷ������λ�õ���ȷ��*/
    {"Please check MQTT Parameter !!!\r\n", dtu_urc_error_mqtt},        /*MQTT��������*/
};

/**
 * @brief       ����DTU�����ϱ���URC��Ϣ���ݣ�ע�⣺����ÿ����һ���ֽ����ݣ�����Ҫͨ��������ڴ������
 * @param       ch:���ڽ��յ�һ���ֽ�����
 * @return      ��
 */
void atk_idm750c_dtu_get_urc_info(uint8_t ch)
{
    static uint8_t ch_last = 0;
    static uint32_t rx_len = 0;
    int i;
    
    /*����DTU����*/
    dtu_rxcmdbuf[rx_len++] = ch;
    
    if (rx_len >= DTU_RX_CMD_BUF_SIZE)
    {
        /*��������*/
        ch_last = 0;
        rx_len = 0;
        memset(dtu_rxcmdbuf, 0, DTU_RX_CMD_BUF_SIZE);
    }
    
    /*����DTU��URC����*/
    if ((ch_last == '\r') && (ch == '\n'))
    {
        for (i = 0; i < ATK_IDM750C_DTU_URC_SIZE; i++)
        {
            if (strstr((char *)dtu_rxcmdbuf, DTU_ATK_M750_URC[i].urc_info) == (char *)dtu_rxcmdbuf)
            {
                DTU_ATK_M750_URC[i].func((char *)dtu_rxcmdbuf, strlen((char *)dtu_rxcmdbuf));
            }
        }
        ch_last = 0;
        rx_len = 0;
        memset(dtu_rxcmdbuf, 0, DTU_RX_CMD_BUF_SIZE);
    }
    
    ch_last = ch;
}

/**
 * @brief       ATK-IDM750C����ATָ��
 * @param       cmd: �����͵�ATָ��
 *              ack: �ȴ�����Ӧ
 *              timeout: �ȴ���ʱʱ��
 * @retval      ATK_IDM750C_EOK: ����ִ�гɹ�
 *              ATK_IDM750C_ETIMEOUT: �ȴ�����Ӧ��ʱ������ִ��ʧ��
 */
uint8_t atk_idm750c_send_cmd_to_dtu(char *cmd, char *ack, uint32_t timeout)
{
    uint8_t *ret = NULL;
    
    if (cmd != NULL)
    {
        atk_idm750c_uart_rx_restart();
        atk_idm750c_uart_printf("%s\r\n", cmd);
    }
    
    if ((ack == NULL) || (timeout == 0))
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        while (timeout > 0)
        {
            ret = atk_idm750c_uart_rx_get_frame();
            if (ret != NULL)
            {
                if (strstr((const char *)ret, ack) != NULL)
                {
                    return ATK_IDM750C_EOK;
                }
                else
                {
                    atk_idm750c_uart_rx_restart();
                }
            }
            timeout--;
            HAL_Delay(1);
        }
        
        return ATK_IDM750C_ETIMEOUT;
    }
}

/**
 * @brief       ATK-IDM750C ATָ�����
 * @param       ��
 * @retval      ATK_IDM750C_EOK: ATָ����Գɹ�
 *              ATK_IDM750C_ERROR: ATָ�����ʧ��
 */
uint8_t atk_idm750c_at_test(void)
{
    uint8_t ret;
    uint8_t i;
    
    for (i=0; i<10; i++)
    {
        ret = atk_idm750c_send_cmd_to_dtu("AT", "OK", ATK_IDM750C_TIME);
        
        if (ret == ATK_IDM750C_EOK)
        {
            return ATK_IDM750C_EOK;
        }
    }
    
    return ATK_IDM750C_ERROR;
}

/**
 * @brief       DTU��������״̬
 * 
 * @param       ��
 * 
 * @return      0  :    �ɹ���������״̬
 *             -1  :    ��������״̬ʧ��
 */
uint8_t atk_idm750c_dtu_enter_configmode(void)
{
    uint8_t ret;
    
    /* ����+++׼����������״̬ */
    atk_idm750c_uart_rx_restart();
    atk_idm750c_uart_printf("+++");
    ret = atk_idm750c_send_cmd_to_dtu(NULL, "atk", 1000);
    if (ret != ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_ERROR;
    }
    
    /* ����atkȷ�Ͻ�������״̬ */
    atk_idm750c_uart_rx_restart();
    atk_idm750c_uart_printf("atk");
    ret = atk_idm750c_send_cmd_to_dtu(NULL, "OK", 1000);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    
    return ATK_IDM750C_ERROR;
}

/**
 * @brief       ATK-IDM750C�ָ���������
 * @param       ��
 * @retval      ATK_IDM750C_EOK  : �ָ��������óɹ�
 *              ATK_IDM750C_ERROR: �ָ���������ʧ��
 */
uint8_t atk_idm750c_restore(void)
{
    uint8_t ret;
    
    ret = atk_idm750c_send_cmd_to_dtu("AT+CONRST", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C��ѯ�汾��
 * @param       ��
 * @retval      ATK_IDM750C_EOK  : ��ѯ�汾�ųɹ�
 *              ATK_IDM750C_ERROR: ��ѯ�汾��ʧ��
 */
uint8_t atk_idm750c_query_versionnumber(void)
{
    uint8_t ret;
    
    ret = atk_idm750c_send_cmd_to_dtu("AT+VERSION", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C��ѯIMEI��
 * @param       ��
 * @retval      ATK_IDM750C_EOK  : ��ѯIMEI��ɹ�
 *              ATK_IDM750C_ERROR: ��ѯIMEI��ʧ��
 */
uint8_t atk_idm750c_query_imeinumber(void)
{
    uint8_t ret;
    
    ret = atk_idm750c_send_cmd_to_dtu("AT+IMEI", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C��ѯICCID��
 * @param       ��
 * @retval      ATK_IDM750C_EOK  : ��ѯICCID��ɹ�
 *              ATK_IDM750C_ERROR: ��ѯICCID��ʧ��
 */
uint8_t atk_idm750c_query_iccidnumber(void)
{
    uint8_t ret;
    
    ret = atk_idm750c_send_cmd_to_dtu("AT+ICCID", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C��ѯIMSI��
 * @param       ��
 * @retval      ATK_IDM750C_EOK  : ��ѯIMSI��ɹ�
 *              ATK_IDM750C_ERROR: ��ѯIMSI��ʧ��
 */
uint8_t atk_idm750c_query_imsinumber(void)
{
    uint8_t ret;
    
    ret = atk_idm750c_send_cmd_to_dtu("AT+IMSI", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C��ѯ��Ʒ���к�
 * @param       ��
 * @retval      ATK_IDM750C_EOK  : ��ѯ��Ʒ���кųɹ�
 *              ATK_IDM750C_ERROR: ��ѯ��Ʒ���к�ʧ��
 */
uint8_t atk_idm750c_query_productid(void)
{
    uint8_t ret;
    
    ret = atk_idm750c_send_cmd_to_dtu("AT+SN", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C����͸��״̬
 * @param       ��
 * @retval      ATK_IDM750C_EOK  : ����͸��״̬�ɹ�
 *              ATK_IDM750C_ERROR: ����͸��״̬ʧ��
 */
uint8_t atk_idm750c_enter_transfermode(void)
{
    uint8_t ret;
    
    ret = atk_idm750c_send_cmd_to_dtu("ATO", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��ָ�����
 * @param       enable:ָ����Ե�ʹ��״̬
 *              ON:ָ�����
 *              OFF:ָ�����
 * @retval      ATK_IDM750C_EOK  : ָ����Գɹ�
 *              ATK_IDM750C_ERROR: ָ�����ʧ��
 */
uint8_t atk_idm750c_commandecho(atk_idm750c_echo_t echo)
{
    uint8_t ret;
    char cmd[30];
    
    switch (echo)
    {
        case ATK_IDM750C_ECHO_ON:
        {
            sprintf(cmd, "AT+R=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_ECHO_OFF:
        {
            sprintf(cmd, "AT+R=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����ģʽ����
 * @param       ��
 * @retval      ATK_IDM750C_EOK  : ����ģʽ���ò��Գɹ�
 *              ATK_IDM750C_ERROR: ����ģʽ���ò���ʧ��
 */
uint8_t atk_idm750c_query_workmode(atk_idm750c_workmode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        case ATK_IDM750C_WORKMODE_NET:       /* ����͸��ģʽ */
        {
            sprintf(cmd, "AT+WORK=\"NET\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_HTTP:      /* HTTP͸��ģʽ */
        {
            sprintf(cmd, "AT+WORK=\"HTTP\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_MQTT:      /* MQTT͸��ģʽ */
        {
            sprintf(cmd, "AT+WORK=\"MQTT\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_ALIYUN:    /* ������͸��ģʽ */
        {
            sprintf(cmd, "AT+WORK=\"ALIYUN\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_ONENET:    /* OneNET͸��ģʽ */
        {
            sprintf(cmd, "AT+WORK=\"ONENET\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_BAUDUYUN:  /* �ٶ���͸��ģʽ */
        {
            sprintf(cmd, "AT+WORK=\"BAIDUYUN\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_RNDIS:     /* ������������ģʽ */
        {
            sprintf(cmd, "AT+WORK=\"RNDIS\"");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ���ڲ���
 * @param       boud:���ڲ�����
 *              stop:����ֹͣλ
 *              data����������λ
 *              parity:������żУ��λ
 * @retval      ATK_IDM750C_EOK  : ���ڲ������ò��Գɹ�
 *              ATK_IDM750C_ERROR: ���ڲ������ò���ʧ��
 */
uint8_t atk_idm750c_uart_parameter(atk_idm750c_uartmode_t mode)
{
    uint8_t ret;
    char cmd[64];
    char *NONE;
    
    switch (mode)
    {
        case ATK_IDM750C_UARTMODE_FIRST:      /* boud��1200��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",1200,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_SECOND:     /* boud��2400��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",2400,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_THIRD:      /* boud��4800��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",4800,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_FOURTH:     /* boud��9600��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",9600,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_FIFTH:      /* boud��14400��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",14400,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_SIXTH:      /* boud��19200��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",19200,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_SEVENTH:    /* boud��38400��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",38400,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_EIGHTH:     /* boud��57600��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",57600,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_NINTH:      /* boud��115200��stop��1��data��8��parity��NONE��Ĭ�ϣ� */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",115200,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_TENTH:      /* boud��128000��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",128000,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_ELEVENTH:   /* boud��230400��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",230400,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_TWELFTH:    /* boud��460800��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",460800,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_THIRTEENTH: /* boud��921600��stop��1��data��8��parity��NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",921600,1,8,NONE);
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_AT_TIMEOUT);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����͸������
 * @param       enable:����͸��ָ���ʹ��״̬
 *              ON:ʹ�ô���͸��ָ��
 *              OFF:��ʹ�ô���͸��ָ��
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_uart(atk_idm750c_urat_t uart)
{
    uint8_t ret;
    char cmd[50];
    
    switch (uart)
    {
        case ATK_IDM750C_UART_ON:
        {
            sprintf(cmd, "AT+UARTAT=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_UART_OFF:
        {
            sprintf(cmd, "AT+UARTAT=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C���ڴ������
 * @param       length�����ڴ�����ȣ� ��Χ�� 1-1024(�ֽ�)
 *              time:���ڴ��ʱ��,��Χ�� 50ms-10000ms
 * @retval      ATK_IDM750C_EOK  : ����͸��״̬�ɹ�
 *              ATK_IDM750C_ERROR: ����͸��״̬ʧ��
 */
uint8_t atk_idm750c_uart_pack_parameter(void)
{
    uint8_t ret;
    char cmd[50];
    
     sprintf(cmd, "AT+UARTLT=\"%d\",\"%d\"",1024,50);
    ret = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����͸��ָ��
 * @param       enable:����͸��ָ���ʹ��״̬
 *              ON:ʹ������͸��ָ��
 *              OFF:��ʹ������͸��ָ��
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_net(atk_idm750c_net_t net)
{
    uint8_t ret;
    char cmd[50];
    
    switch (net)
    {
        case ATK_IDM750C_UART_ON:
        {
            sprintf(cmd, "AT+NETAT=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_UART_OFF:
        {
            sprintf(cmd, "AT+NETAT=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ͸��ָ����Կ
 * @param       data:ָ����Կ���ݣ�������� 20 ���ֽ�
 *              data Ĭ�ϲ����� ALIENTEK
 * @retval      ATK_IDM750C_EOK  : ͸��ָ����Կ���óɹ�
 *              ATK_IDM750C_ERROR: ͸��ָ����Կ����ʧ��
 */
uint8_t atk_idm750c_cmdkey(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+CMDKEY=\"%s\"", "ALIENTEK");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �û��Զ�������
 * @param       data:�û��Զ������ݣ�������� 32 ���ֽ�
 *              data Ĭ�ϲ����� ALIENTEK
 * @retval      ATK_IDM750C_EOK  : �û��Զ����������óɹ�
 *              ATK_IDM750C_ERROR: �û��Զ�����������ʧ��
 */
uint8_t atk_idm750c_user(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+USER=\"%s\"", "ALIENTEK");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ���������ݴ����Զ�����ʱ��
 * @param       time:�Զ�����ʱ��,ʱ�����÷�Χ�� 60s-3600s
 *              time Ĭ�ϲ����� 1200
 * @retval      ATK_IDM750C_EOK  : ���������ݴ����Զ�����ʱ�����óɹ�
 *              ATK_IDM750C_ERROR: ���������ݴ����Զ�����ʱ������ʧ��
 */
uint8_t atk_idm750c_rstime(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+RSTIM=\"%s\"", "1200");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �������ӳ�ʱ����ʱ��
 * @param       time:��ʱ����ʱ�䣬��Χ�� 30s-6000s
 *              time Ĭ�ϲ����� 120s
 * @retval      ATK_IDM750C_EOK  : �������ӳ�ʱ����ʱ�����óɹ�
 *              ATK_IDM750C_ERROR: �������ӳ�ʱ����ʱ������ʧ��
 */
uint8_t atk_idm750c_link_rstime(void)
{
    uint8_t ret;
    char cmd[30];
    
    sprintf(cmd, "AT+LINKRSTM=\"%s\"", "120");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����Ӣ�Ķ���
 * @param       phone:Ŀ��绰���룬������� 20 ���ֽ�
 *              data�� �������ݣ�������� 160 ���ֽ�
 * @retval      ATK_IDM750C_EOK  : ����Ӣ�Ķ��ųɹ�
 *              ATK_IDM750C_ERROR: ����Ӣ�Ķ���ʧ��
 */
uint8_t atk_idm750c_send_english_message(void)
{
    uint8_t ret;
    char cmd[50];
    char *ALIENTEK;
    
    sprintf(cmd, "AT+SMSEND=\"%d\",\"%s\"", 123456789, ALIENTEK);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��һ·���Ӳ���
 * @param       TCP:TCP ģʽ
 *              UDP:UDP ģʽ
 *              ip�� ��������ַ��IP ��ַ��������֧�֣� ,������� 100 ���ֽ�
 *              port���������˿ںţ���С��Χ 0-65535
 *              ��һ·����Ĭ�ϲ����� AT+LINK1="TCP","cloud.alientek.com","59666"
 * @retval      ATK_IDM750C_EOK  : ��һ·���Ӳ������óɹ�
 *              ATK_IDM750C_ERROR: ��һ·���Ӳ�������ʧ��
 */
uint8_t atk_idm750c_link1(atk_idm750c_link1mode_t link1, char *link1_domainname, char *link1_port)
{
    uint8_t ret;
    char cmd[64];
    
    switch (link1)
    {
        case ATK_IDM750C_LINK1MODE_TCP:
        {
            sprintf(cmd, "AT+LINK1=\"TCP\",\"%s\",\"%s\"",link1_domainname, link1_port);
            break;
        }
        case ATK_IDM750C_LINK1MODE_UDP:
        {
            sprintf(cmd, "AT+LINK1=\"UDP\",\"%s\",\"%s\"",link1_domainname, link1_port);
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ڶ�·���Ӳ���
 * @param       TCP:TCP ģʽ
 *              UDP:UDP ģʽ
 *              ip�� ��������ַ��IP ��ַ��������֧�֣� ,������� 100 ���ֽ�
 *              port���������˿ںţ���С��Χ 0-65535
 *              �ڶ�·����Ĭ�ϲ����� AT+LINK1="TCP","cloud.alientek.com","59666"
 * @retval      ATK_IDM750C_EOK  : �ڶ�·���Ӳ������óɹ�
 *              ATK_IDM750C_ERROR: �ڶ�·���Ӳ�������ʧ��
 */
uint8_t atk_idm750c_link2(atk_idm750c_link2mode_t link2, char *link2_domainname, char *link2_port)
{
    uint8_t ret;
    char cmd[64];
    
    switch (link2)
    {
        case ATK_IDM750C_LINK2MODE_TCP:
        {
            sprintf(cmd, "AT+LINK2=\"TCP\",\"%s\",\"%s\"",link2_domainname, link2_port);
            break;
        }
        case ATK_IDM750C_LINK2MODE_UDP:
        {
            sprintf(cmd, "AT+LINK2=\"UDP\",\"%s\",\"%s\"",link2_domainname, link2_port);
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·���Ӳ���
 * @param       TCP:TCP ģʽ
 *              UDP:UDP ģʽ
 *              ip�� ��������ַ��IP ��ַ��������֧�֣� ,������� 100 ���ֽ�
 *              port���������˿ںţ���С��Χ 0-65535
 *              ����·����Ĭ�ϲ����� AT+LINK1="TCP","cloud.alientek.com","59666"
 * @retval      ATK_IDM750C_EOK  : ����·���Ӳ������óɹ�
 *              ATK_IDM750C_ERROR: ����·���Ӳ�������ʧ��
 */
uint8_t atk_idm750c_link3(atk_idm750c_link3mode_t link3, char *link3_domainname, char *link3_port)
{
    uint8_t ret;
    char cmd[64];
    
    switch (link3)
    {
        case ATK_IDM750C_LINK3MODE_TCP:
        {
            sprintf(cmd, "AT+LINK3=\"TCP\",\"%s\",\"%s\"",link3_domainname, link3_port);
            break;
        }
        case ATK_IDM750C_LINK3MODE_UDP:
        {
            sprintf(cmd, "AT+LINK3=\"UDP\",\"%s\",\"%s\"",link3_domainname, link3_port);
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·���Ӳ���
 * @param       TCP:TCP ģʽ
 *              UDP:UDP ģʽ
 *              ip�� ��������ַ��IP ��ַ��������֧�֣� ,������� 100 ���ֽ� 
 *              port���������˿ںţ���С��Χ 0-65535
 *              ����·����Ĭ�ϲ����� AT+LINK1="TCP","cloud.alientek.com","59666"
 * @retval      ATK_IDM750C_EOK  : ����·���Ӳ������óɹ�
 *              ATK_IDM750C_ERROR: ����·���Ӳ�������ʧ��
 */
uint8_t atk_idm750c_link4(atk_idm750c_link4mode_t link4, char *link4_domainname, char *link4_port)
{
    uint8_t ret;
    char cmd[64];
    
    switch (link4)
    {
        case ATK_IDM750C_LINK4MODE_TCP:
        {
            sprintf(cmd, "AT+LINK4=\"TCP\",\"%s\",\"%s\"",link4_domainname, link4_port);
            break;
        }
        case ATK_IDM750C_LINK4MODE_UDP:
        {
            sprintf(cmd, "AT+LINK4=\"UDP\",\"%s\",\"%s\"",link4_domainname, link4_port);
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��һ·����ʹ��״̬
 * @param       enable:����ʹ��״̬
 *              ON:��������
 *              OFF:����������
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_link1en(atk_idm750c_link1en_t link1en)
{
    uint8_t ret;
    char cmd[50];
    
    switch (link1en)
    {
        case ATK_IDM750C_LINK1EN_ON:
        {
            sprintf(cmd, "AT+LINK1EN=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_LINK1EN_OFF:
        {
            sprintf(cmd, "AT+LINK1EN=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ڶ�·����ʹ��״̬
 * @param       enable:����ʹ��״̬
 *              ON:��������
 *              OFF:����������
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_link2en(atk_idm750c_link2en_t link2en)
{
    uint8_t ret;
    char cmd[50];
    
    switch (link2en)
    {
        case ATK_IDM750C_LINK2EN_ON:
        {
            sprintf(cmd, "AT+LINK2EN=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_LINK2EN_OFF:
        {
            sprintf(cmd, "AT+LINK2EN=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·����ʹ��״̬
 * @param       enable:����ʹ��״̬
 *              ON:��������
 *              OFF:����������
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_link3en(atk_idm750c_link3en_t link3en)
{
    uint8_t ret;
    char cmd[50];
    
    switch (link3en)
    {
        case ATK_IDM750C_LINK3EN_ON:
        {
            sprintf(cmd, "AT+LINK3EN=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_LINK2EN_OFF:
        {
            sprintf(cmd, "AT+LINK3EN=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·����ʹ��״̬
 * @param       enable:����ʹ��״̬
 *              ON:��������
 *              OFF:����������
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_link4en(atk_idm750c_link4en_t link4en)
{
    uint8_t ret;
    char cmd[50];
    
    switch (link4en)
    {
        case ATK_IDM750C_LINK4EN_ON:
        {
            sprintf(cmd, "AT+LINK4EN=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_LINK4EN_OFF:
        {
            sprintf(cmd, "AT+LINK4EN=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��һ·����ģʽ
 * @param       mode:����ģʽ
 *              LONG:�����ӣ�Ĭ�ϣ�
 *              SHORT:������
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_link1md(atk_idm750c_link1md_t link1md)
{
    uint8_t ret;
    char cmd[50];
    
    switch (link1md)
    {
        case ATK_IDM750C_LINK1MD_LONG:
        {
            sprintf(cmd, "AT+LINK1MD=\"%s\"", "LONG");
            break;
        }
        case ATK_IDM750C_LINK1MD_SHORT:
        {
            sprintf(cmd, "AT+LINK1MD=\"%s\"", "SHORT");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ڶ�·����ģʽ
 * @param       mode:����ģʽ
 *              LONG:�����ӣ�Ĭ�ϣ�
 *              SHORT:������
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_link2md(atk_idm750c_link2md_t link2md)
{
    uint8_t ret;
    char cmd[50];
    
    switch (link2md)
    {
        case ATK_IDM750C_LINK2MD_LONG:
        {
            sprintf(cmd, "AT+LINK2MD=\"%s\"", "LONG");
            break;
        }
        case ATK_IDM750C_LINK2MD_SHORT:
        {
            sprintf(cmd, "AT+LINK2MD=\"%s\"", "SHORT");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·����ģʽ
 * @param       mode:����ģʽ
 *              LONG:�����ӣ�Ĭ�ϣ�
 *              SHORT:������
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_link3md(atk_idm750c_link3md_t link3md)
{
    uint8_t ret;
    char cmd[50];
    
    switch (link3md)
    {
        case ATK_IDM750C_LINK3MD_LONG:
        {
            sprintf(cmd, "AT+LINK1MD=\"%s\"", "LONG");
            break;
        }
        case ATK_IDM750C_LINK3MD_SHORT:
        {
            sprintf(cmd, "AT+LINK1MD=\"%s\"", "SHORT");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·����ģʽ
 * @param       mode:����ģʽ
 *              LONG:�����ӣ�Ĭ�ϣ�
 *              SHORT:������
 * @retval      ATK_IDM750C_EOK  : ����͸�����óɹ�
 *              ATK_IDM750C_ERROR: ����͸������ʧ��
 */
uint8_t atk_idm750c_link4md(atk_idm750c_link4md_t link4md)
{
    uint8_t ret;
    char cmd[50];
    
    switch (link4md)
    {
        case ATK_IDM750C_LINK4MD_LONG:
        {
            sprintf(cmd, "AT+LINK4MD=\"%s\"", "LONG");
            break;
        }
        case ATK_IDM750C_LINK4MD_SHORT:
        {
            sprintf(cmd, "AT+LINK4MD=\"%s\"", "SHORT");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��ѯ��һ·����״̬
 * @param       state����������״̬
 *              CONNECTED��������
 *              DISCONNECTED��δ����
 * @retval      ATK_IDM750C_EOK  : ��ѯ�ɹ�
 *              ATK_IDM750C_ERROR: ��ѯʧ��
 */
uint8_t atk_idm750c_query_link1sta(void)
{
    uint8_t ret;
    
    ret  = atk_idm750c_send_cmd_to_dtu("AT+LINK1STA", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��ѯ�ڶ�·����״̬
 * @param       state����������״̬
 *              CONNECTED��������
 *              DISCONNECTED��δ����
 * @retval      ATK_IDM750C_EOK  : ��ѯ�ɹ�
 *              ATK_IDM750C_ERROR: ��ѯʧ��
 */
uint8_t atk_idm750c_query_link2sta(void)
{
    uint8_t ret;
    
    ret  = atk_idm750c_send_cmd_to_dtu("AT+LINK2STA", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��ѯ����·����״̬
 * @param       state����������״̬
 *              CONNECTED��������
 *              DISCONNECTED��δ����
 * @retval      ATK_IDM750C_EOK  : ��ѯ�ɹ�
 *              ATK_IDM750C_ERROR: ��ѯʧ��
 */
uint8_t atk_idm750c_query_link3sta(void)
{
    uint8_t ret;
    
    ret  = atk_idm750c_send_cmd_to_dtu("AT+LINK3STA", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��ѯ����·����״̬
 * @param       state����������״̬
 *              CONNECTED��������
 *              DISCONNECTED��δ����
 * @retval      ATK_IDM750C_EOK  : ��ѯ�ɹ�
 *              ATK_IDM750C_ERROR: ��ѯʧ��
 */
uint8_t atk_idm750c_query_link4sta(void)
{
    uint8_t ret;
    
    ret  = atk_idm750c_send_cmd_to_dtu("AT+LINK4STA", "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��һ·��ʱ����ʱ��
 * @param       time����ʱ����ʱ�䣬��Χ1s-60s
 *              timeĬ��ֵΪ��5s
 * @retval      ATK_IDM750C_EOK  : �����ɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_link1_timeout(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+LINK1TM=\"%d\"", 5);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ڶ�·��ʱ����ʱ��
 * @param       time����ʱ����ʱ�䣬��Χ1s-60s
 *              timeĬ��ֵΪ��5s
 * @retval      ATK_IDM750C_EOK  : �����ɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_link2_timeout(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+LINK2TM=\"%d\"", 5);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·��ʱ����ʱ��
 * @param       time����ʱ����ʱ�䣬��Χ1s-60s
 *              timeĬ��ֵΪ��5s
 * @retval      ATK_IDM750C_EOK  : �����ɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_link3_timeout(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+LINK3TM=\"%d\"", 5);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·��ʱ����ʱ��
 * @param       time����ʱ����ʱ�䣬��Χ1s-60s
 *              timeĬ��ֵΪ��5s
 * @retval      ATK_IDM750C_EOK  : �����ɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_link4_timeout(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+LINK4TM=\"%d\"", 5);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ��һ·�����ӳ���ʱ��
 * @param       time�����ӳ�ʱʱ�䣬��Χ1s-100s
 *              timeĬ��ֵΪ��10s
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_link1_duration(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+LINK1SOT=\"%d\"", 10);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ڶ�·�����ӳ���ʱ��
 * @param       time�����ӳ�ʱʱ�䣬��Χ1s-100s
 *              timeĬ��ֵΪ��10s
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_link2_duration(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+LINK2SOT=\"%d\"", 10);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·�����ӳ���ʱ��
 * @param       time�����ӳ�ʱʱ�䣬��Χ1s-100s
 *              timeĬ��ֵΪ��10s
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_link3_duration(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+LINK3SOT=\"%d\"", 10);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����·�����ӳ���ʱ��
 * @param       time�����ӳ�ʱʱ�䣬��Χ1s-100s
 *              timeĬ��ֵΪ��10s
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_link4_duration(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+LINK4SOT=\"%d\"", 10);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C APN����
 * @param       apn:APN���ͣ��������98���ֽ�
 *              ����Ϊͨ�ý���㣺
 *              �Զ���ȡAPN����㣺AUTO���й��ƶ�����㣺CMNET
 *              �й���ͨ����㣺UNINET���й����Ž���㣺CTNET
 *              user name���û�������û�б㲻��Ҫ��д���������30���ֽ�
 *              password�����룬��û�б㲻��Ҫ��д���������30���ֽ�
 *              CONNECTED��������
 *              DISCONNECTED��δ����
 * @retval      ATK_IDM750C_EOK  : ����Ӣ�Ķ��ųɹ�
 *              ATK_IDM750C_ERROR: ����Ӣ�Ķ���ʧ��
 */
uint8_t atk_idm750c_set_apn(void)
{
    uint8_t ret;
    char cmd[50];
    char *ALIENTEK;
    
    sprintf(cmd, "AT+APN=\"AUTO\",\"%s\",\"%d\"", ALIENTEK, 123456789);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ָ�����ݷ��͵�һ·����
 * @param       socket���������ݵ���һ·���ӣ�������Χ��1-4
 *              data����Ҫ���͵�����
 *              ALIENTEK��͸��ָ����Կ������ͨ�� AT ָ�������λ���޸ģ�����Ĭ��Ϊ�� ALIENTEK��
 *              1��Ϊ��һ·����
 *              alientek��͸�����ݣ����ź��������ȫ��Ϊ͸�����ݣ�
 * @retval      ATK_IDM750C_EOK  : ���ͳɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_send_data(void)
{
    uint8_t ret;
    char cmd[50];
    char *alientek;
    
    sprintf(cmd, "ALIENTEK@AT+SEND=\"%d\",\"%s\"", 1, alientek);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����͸��ģʽ���ݷ���ģʽ
 * @param       enable ���ݷ���ģʽ:
 *              ON : �췢ģʽ
 *              OFF������ģʽ
 *              Ĭ��Ϊ�췢ģʽ�� +SENDFAST: "ON"
 * @retval      ATK_IDM750C_EOK  : ���ͳɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_send_fast(atk_idm750c_sendfast_t sendfast)
{
    uint8_t ret;
    char cmd[50];
    
    switch (sendfast)
    {
        case ATK_IDM750C_SENDFAST_ON:
        {
            sprintf(cmd, "AT+SENDFAST=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_SENDFAST_OFF:
        {
            sprintf(cmd, "AT+SENDFAST=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ԭ�������ָ��
 * @param       enable ����״̬
 *              ON : ����
 *              OFF��������
 *              Ĭ�ϲ�����OFF
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_set_yuanziyun(atk_idm750c_set_yuanziyun_t yuanziyun)
{
    uint8_t ret;
    char cmd[50];
    
    switch (yuanziyun)
    {
        case ATK_IDM750C_SET_YUANZIYUN_ON:
        {
            sprintf(cmd, "AT+SVREN=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_SET_YUANZIYUN_OFF:
        {
            sprintf(cmd, "AT+SVREN=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ԭ�����豸���
 * @param       data�� �豸��ţ��̶�Ϊ20���ֽڣ���Ϊ������
 *              dataĬ�ϲ�����12345678901234567890���������
 *              ����ԭ�������ɵ��豸���Ϊ׼
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_device_num(char *devicenum)
{
    uint8_t ret;
    char cmd[64];
    
    sprintf(cmd, "AT+SVRNUM=\"%s\"",devicenum);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_SET_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ԭ�����豸����
 * @param       data���豸���룬�̶�8���ֽڣ���Ϊ������
 *              Ĭ�ϲ�����12345678,�û�Ҳ������������
 * @retval      ATK_IDM750C_EOK  : ���ͳɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_device_password(char *devicepassword)
{
    uint8_t ret;
    char cmd[64];
    
    sprintf(cmd, "AT+SVRKEY=\"%s\"",devicepassword);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ע�������״̬
 * @param       enable ����״̬
 *              ON : ����
 *              OFF��������
 *              Ĭ�ϲ�����OFF
 * @retval      ATK_IDM750C_EOK  : ���ͳɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_registration_package(atk_idm750c_regen_t regen)
{
    uint8_t ret;
    char cmd[50];
    
    switch (regen)
    {
        case ATK_IDM750C_SET_REGEN_ON:
        {
            sprintf(cmd, "AT+REGEN=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_SET_REGEN_OFF:
        {
            sprintf(cmd, "AT+REGEN=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ע�������
 * @param       data:ע������ݣ� ������� 100 ���ֽڣ�
 *              ������ʮ�������ַ�������Ϊż���� ���㲹�㡣
 *              data Ĭ�ϲ����� 414C49454E54454B2D5245474454
 * @retval      ATK_IDM750C_EOK  : ���ͳɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_registration_package_data(void)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+REGDT=\"414C49454E54454B2D5245474454\"");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ע������ͷ�ʽ
 * @param       mode:ע�������ģʽ
 *              LINK:�ɹ����ӽ�������һ��
 *              DATA:ÿ֡����ǰ���ͷ���һ��
 *              ALL��ͬʱ֧���������ַ���ģʽ
 *              mode Ĭ�ϲ����� LINK
 * @retval      ATK_IDM750C_EOK  : ���ͳɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_registration_package_send_method(void)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+REGMD=\"LINK\"");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ע�����������
 * @param       type:ע�����������
                IMEI:�� IMEI ����ע�������
                ICCID:�� ICCID ����ע�������
                IMSI:�� IMSI ����ע�������
                USER�� �û��Զ���ע������ݣ�ͨ�� AT+REGDT ָ������
                type Ĭ�ϲ����� IMEI
 * @retval      ATK_IDM750C_EOK  : ���ͳɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_registration_package_data_type(void)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+REGTP=\"IMEI\"");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����������״̬
 * @param       enable ����״̬
 *              ON : ����
 *              OFF��������
 *              Ĭ�ϲ�����OFF
 * @retval      ATK_IDM750C_EOK  : �����ɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_heartbeat_package(atk_idm750c_hrten_t heart)
{
    uint8_t ret;
    char cmd[50];
    
    switch (heart)
    {
        case ATK_IDM750C_HRTEN_ON:
        {
            sprintf(cmd, "AT+HRTEN=\"%s\"", "ON");
            break;
        }
        case ATK_IDM750C_HRTEN_OFF:
        {
            sprintf(cmd, "AT+HRTEN=\"%s\"", "OFF");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ����������
 * @param       data:���������ݣ� ������� 100 ���ֽڣ�
 *              ������ʮ�������ַ�������Ϊż���� ���㲹�㡣
 *              data Ĭ�ϲ����� 414C49454E54454B2D4852544454
 * @retval      ATK_IDM750C_EOK  : ���ͳɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_heartbeat_package_data(void)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+HRTDT=\"414C49454E54454B2D4852544454\"");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ������ʱ����
 * @param       time:��λ�룬��Χ�� 5s-1200s
 *              time Ĭ�ϲ����� 120s
 * @retval      ATK_IDM750C_EOK  : ������ʱ�������óɹ�
 *              ATK_IDM750C_ERROR: ������ʱ��������ʧ��
 */
uint8_t atk_idm750c_heartbeat_package_interval(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+HRTTM=\"%s\"", "120");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C HTTP���ָ��
 * @param       mode:HTTP ����ʽ
 *              GET: GET ��ʽ
 *              POST:POST ��ʽ
 *              mode Ĭ�ϲ����� GET
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_http_request_mode(atk_idm750c_http_request_mode_t mode)
{
    uint8_t ret;
    char cmd[50];
    
    switch (mode)
    {
        case ATK_IDM750C_HTTP_REQUEST_GET:
        {
            sprintf(cmd, "AT+HTTPMD=\"%s\"", "GET");
            break;
        }
        case ATK_IDM750C_HTTP_REQUEST_POST:
        {
            sprintf(cmd, "AT+HTTPMD=\"%s\"", "POST");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C HTTP URL��ַ
 * @param       ������
 *              data:URL ��ַ��������� 200 ���ֽڡ�
 *              data Ĭ�ϲ���: AT+HTTPURL=" https://cloud.alientek.com/testfordtu?data="
 * @retval      ATK_IDM750C_EOK  : ��ַ���óɹ�
 *              ATK_IDM750C_ERROR: ��ַ����ʧ��
 */
uint8_t atk_idm750c_http_url_address(char *http_address)
{
    uint8_t ret;
    char cmd[200];
    
    sprintf(cmd, "AT+HTTPURL=\"%s\"",http_address);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C HTTP����ʱʱ��
 * @param       time:����ʱʱ�䣬��Χ�� 5s-60s
 *              time Ĭ�ϲ����� 10s
 * @retval      ATK_IDM750C_EOK  : HTTP����ʱʱ������óɹ�
 *              ATK_IDM750C_ERROR: HTTP����ʱʱ�������ʧ��
 */
uint8_t atk_idm750c_http_request_timeout(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+HTTPTM=\"10\"");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C HTTP����ͷ��Ϣ
 * @param       header:����ͷ��Ϣ�� ���ƴ�С 200 �ֽڡ�
 *              header Ĭ�ϲ���: AT+HTTPHD="Connection:close"
 * @retval      ATK_IDM750C_EOK  : HTTP����ͷ��Ϣ���óɹ�
 *              ATK_IDM750C_ERROR: HTTP����ͷ��Ϣ����ʧ��
 */
uint8_t atk_idm750c_http_request_header_information(void)
{
    uint8_t ret;
    char cmd[50];
    
    sprintf(cmd, "AT+HTTPHD=\"Connection:close\"");
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C MQTT clientid ��Ϣ
 * @param       ������
 *              clientid:MQTT �� clientid ��Ϣ������ 200 ���ֽڡ�
 *              clientid Ĭ�ϲ���: AT+MQTTCD="alientek"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_mqtt_clientid(char *clientid)
{
    uint8_t ret;
    char cmd[10];
    
    sprintf(cmd, "AT+MQTTCD=\"%s\"",clientid);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C MQTT username ��Ϣ
 * @param       ������
 *              clientid:MQTT �� username ��Ϣ������ 200 ���ֽڡ�
 *              clientid Ĭ�ϲ���: AT+MQTTUN="admin"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_mqtt_username(char *username)
{
    uint8_t ret;
    char cmd[10];
    
    sprintf(cmd, "AT+MQTTUN=\"%s\"",username);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C MQTT password ��Ϣ
 * @param       ������
 *              clientid:MQTT �� password ��Ϣ������ 200 ���ֽڡ�
 *              clientid Ĭ�ϲ���: AT+MQTTUN="password"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_mqtt_password(char *password)
{
    uint8_t ret;
    char cmd[10];
    
    sprintf(cmd, "AT+MQTTPW=\"%s\"", password);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C MQTT ��������Ϣ
 * @param       ������
 *              ip:MQTT ��������ַ������ 128 ���ֽڡ� ������ IP ��ַҲ����Ϊ����
 *              port:MQTT �������˿ںţ� ���� 0-65535��
 *              Ĭ�ϲ���: AT+MQTTIP="cloud.alientek.com","1883"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_server_address(char *ip,char *port)
{
    uint8_t ret;
    char cmd[128];
    
    sprintf(cmd, "AT+MQTTIP=\"%s\",\"%s\"", ip, port);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C MQTT ����������Ϣ
 * @param       ������
 *              sub:����������Ϣ�� ���ƴ�С 100 �ֽڡ�
 *              Ĭ�ϲ���: AT+MQTTSUB="atk/sub"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_mqtt_sub_title(char *title)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+MQTTSUB=\"%s\"", title);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C MQTT ����������Ϣ
 * @param       ������
 *              pub:����������Ϣ�� ���ƴ�С 100 �ֽڡ�
 *              Ĭ�ϲ���: AT+MQTTPUB="atk/pub"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_mqtt_pub_title(char *title)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+MQTTPUB=\"%s\"", title);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C MQTT ������Ϣ
 * @param       ������
 *              qos����Ϣ����ȼ�
 *              0:��෢��һ��
 *              1:���ٷ���һ��
 *              2:ֻ����һ��
 *              retain:��������״̬
 *              0:���������ᱣ��������Ϣ
 *              1:����������������Ϣ
 *              clean_session:����Ự״̬
 *              0: �ͻ������ߺ� ����������Ự��Ϣ
 *              1: �ͻ������ߺ� �������Զ��ݻ���������
 *              keepalive:����ʱ��,��Χ�� 1-65535�� ��λ���롣
 *              Ĭ�ϲ���: AT+MQTTCON="0","0","1","300"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_mqtt_para_information(atk_idm750c_mqtt_para_mode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        /* 0����෢��һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_MQTT_ZERO_MODE1:
        {
            sprintf(cmd, "AT+MQTTCON=\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0����෢��һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� (Ĭ��)*/
        case ATK_IDM750C_MQTT_ZERO_MODE2:
        {
            sprintf(cmd, "AT+MQTTCON=\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0����෢��һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_MQTT_ZERO_MODE3:
        {
            sprintf(cmd, "AT+MQTTCON=\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0����෢��һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_MQTT_ZERO_MODE4:
        {
            sprintf(cmd, "AT+MQTTCON=\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1�����ٷ���һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_MQTT_ONE_MODE1:
        {
            sprintf(cmd, "AT+MQTTCON=\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1�����ٷ���һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_MQTT_ONE_MODE2:
        {
            sprintf(cmd, "AT+MQTTCON=\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1�����ٷ���һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_MQTT_ONE_MODE3:
        {
            sprintf(cmd, "AT+MQTTCON=\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1�����ٷ���һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_MQTT_ONE_MODE4:
        {
            sprintf(cmd, "AT+MQTTCON=\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 2��ֻ����һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_MQTT_TWO_MODE1:
        {
            sprintf(cmd, "AT+MQTTCON=\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 2��ֻ����һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_MQTT_TWO_MODE2:
        {
            sprintf(cmd, "AT+MQTTCON=\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 2��ֻ����һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_MQTT_TWO_MODE3:
        {
            sprintf(cmd, "AT+MQTTCON=\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 2��ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_MQTT_TWO_MODE4:
        {
            sprintf(cmd, "AT+MQTTCON=\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ������ProductKey��Ϣ
 * @param       ������
 *              pk:������ ProductKey ��Ϣ�� ���ƴ�С 20 �ֽڡ�
 *              Ĭ�ϲ���: AT+ALIPK="ProductKey"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_aliyun_productkey(char *key)
{
    uint8_t ret;
    char cmd[20];
    
    sprintf(cmd, "AT+ALIPK=\"%s\"", key);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ������DeviceSecret��Ϣ
 * @param       ������
 *              ds:������ DeviceSecret ��Ϣ�� ���ƴ�С 64 �ֽڡ�
 *              Ĭ�ϲ���: AT+ALIDS="DeviceSecret
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_aliyun_devicesecret(char *secret)
{
    uint8_t ret;
    char cmd[64];
    
    sprintf(cmd, "AT+ALIDS=\"%s\"", secret);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ������DeviceName��Ϣ
 * @param       ������
 *              dn: ������ DeviceName ��Ϣ�� ���ƴ�С 32 �ֽڡ�
 *              Ĭ�ϲ���: AT+ALIDN="DeviceName"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_aliyun_devicename(char *name)
{
    uint8_t ret;
    char cmd[32];
    
    sprintf(cmd, "AT+ALIDN=\"%s\"", name);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �����Ƶ�����Ϣ
 * @param       ������
 *              ri: �����Ƶ�����Ϣ�� ���ƴ�С 20 �ֽڡ�
 *              Ĭ�ϲ���: AT+ALIRI="cn-shanghai
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_aliyun_region(char *reg)
{
    uint8_t ret;
    char cmd[20];
    
    sprintf(cmd, "AT+ALIRI=\"%s\"", reg);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �����ƶ���������Ϣ
 * @param       ������
 *              sub:����������Ϣ�� ���ƴ�С 100 �ֽڡ�
 *              Ĭ�ϲ���: AT+ALISUB="get"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_aliyun_subscribe_title(char *asti)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+ALISUB=\"%s\"", asti);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �����Ʒ���������Ϣ
 * @param       ������
 *              pub:����������Ϣ�� ���ƴ�С 100 �ֽڡ�
 *              Ĭ�ϲ���: AT+ALIPUB="update"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_aliyun_release_title(char *arti)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+ALIPUB=\"%s\"", arti);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �����Ʋ�����Ϣ
 * @param       ������
 *              qos����Ϣ����ȼ�
 *              0:��෢��һ��
 *              1:���ٷ���һ��
 *              2:ֻ����һ��
 *              retain:��������״̬
 *              0:���������ᱣ��������Ϣ
 *              1:����������������Ϣ
 *              clean_session:����Ự״̬
 *              0: �ͻ������ߺ� ����������Ự��Ϣ
 *              1: �ͻ������ߺ� �������Զ��ݻ���������
 *              keepalive:����ʱ��,��Χ�� 30-1200����λ���롣 ��
 *              Ĭ�ϲ���: AT+ALICON="0","0","1","300"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_aliyun_para_information(atk_idm750c_aliyun_para_mode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        /* 0����෢��һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ALIYUN_ZERO_MODE1:
        {
            sprintf(cmd, "AT+ALICON=\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0����෢��һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� ��Ĭ�ϣ�*/
        case ATK_IDM750C_ALIYUN_ZERO_MODE2:
        {
            sprintf(cmd, "AT+ALICON=\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0����෢��һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ALIYUN_ZERO_MODE3:
        {
            sprintf(cmd, "AT+ALICON=\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0����෢��һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ALIYUN_ZERO_MODE4:
        {
            sprintf(cmd, "AT+ALICON=\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1�����ٷ���һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ALIYUN_ONE_MODE1:
        {
            sprintf(cmd, "AT+ALICON=\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1�����ٷ���һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ALIYUN_ONE_MODE2:
        {
            sprintf(cmd, "AT+ALICON=\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1�����ٷ���һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ALIYUN_ONE_MODE3:
        {
            sprintf(cmd, "AT+ALICON=\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1�����ٷ���һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ALIYUN_ONE_MODE4:
        {
            sprintf(cmd, "AT+ALICON=\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 2��ֻ����һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ALIYUN_TWO_MODE1:
        {
            sprintf(cmd, "AT+ALICON=\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 2��ֻ����һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ALIYUN_TWO_MODE2:
        {
            sprintf(cmd, "AT+ALICON=\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 2��ֻ����һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ALIYUN_TWO_MODE3:
        {
            sprintf(cmd, "AT+ALICON=\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 2��ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ALIYUN_TWO_MODE4:
        {
            sprintf(cmd, "AT+ALICON=\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C OneNET �豸ID��Ϣ
 * @param       ������
 *              di�� �豸 ID ��Ϣ������ 20 �ֽڡ�
 *              Ĭ�ϲ���: AT+ONEDI="12345"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_onenet_device_id(char *id)
{
    uint8_t ret;
    char cmd[20];
    
    sprintf(cmd, "AT+ONEDI=\"%s\"", id);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C OneNET ��ƷID��Ϣ
 * @param       ������
 *              pi�� ��Ʒ ID ��Ϣ������ 20 �ֽڡ�
 *              Ĭ�ϲ���: AT+ONEPI="1234567890"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_onenet_product_id(char *id)
{
    uint8_t ret;
    char cmd[20];
    
    sprintf(cmd, "AT+ONEPI=\"%s\"", id);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C OneNET ��Ȩ��Ϣ
 * @param       ������
 *              ai�� ��Ȩ��Ϣ��Ϣ������ 64 �ֽڡ�
 *              Ĭ�ϲ���: AT+ONEAI="12345678901234567890"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_onenet_authentication_information(char *ai)
{
    uint8_t ret;
    char cmd[64];
    
    sprintf(cmd, "AT+ONEAI=\"%s\"", ai);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C OneNET ��������Ϣ
 * @param       ������
 *              ip�� ��������ַ�� ���� 40 �ֽڡ�
 *              port�� �������˿ںţ� ��Χ�� 0-65535
 *              Ĭ�ϲ���: AT+ONEIP="mqtt.heclouds.com","6002"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_onenet_server_information(char *ip,char *port)
{
    uint8_t ret;
    char cmd[40];
    
    sprintf(cmd, "AT+ONEIP=\"%s\",\"%s\"", ip, port);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ONENET������Ϣ
 * @param       ������
 *              data_type:�������ͣ�ע�� ��ο� OneNET �豸�˽���Э��-MQTT �ĵ��е��������Ͷ��壩
 *              1�� �������� 1
 *              3�� �������� 3
 *              4�� �������� 4
 *              qos����Ϣ����ȼ�
 *              0:��෢��һ��
 *              1:���ٷ���һ��
 *              2:ֻ����һ��
 *              retain:��������״̬
 *              0:���������ᱣ��������Ϣ
 *              1:����������������Ϣ
 *              clean_session:����Ự״̬
 *              0:�ͻ������ߺ� ����������Ự��Ϣ
 *              1:�ͻ������ߺ󣬷������Զ��ݻ���������
 *              keepalive:����ʱ��,��Χ�� 120-1800����λ���롣
 *              Ĭ�ϲ���: AT+ONECON="1","0","0","1","300"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_onenet_para_information(atk_idm750c_onenet_para_mode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        /* 1:�������� 1��0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_ZERO_MODE1:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1:�������� 1��0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������ݣ�Ĭ�ϣ� */
        case ATK_IDM750C_ONENET_ZERO_MODE2:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1:�������� 1��0:��෢��һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_ZERO_MODE3:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1:�������� 1��0:��෢��һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ZERO_MODE4:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1:�������� 1��1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_ZERO_MODE5:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1:�������� 1��1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ZERO_MODE6:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1:�������� 1��1:���ٷ���һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_ZERO_MODE7:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1:�������� 1��1:���ٷ���һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ZERO_MODE8:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1:�������� 1��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ZERO_MODE9:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1:�������� 1��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ZERO_MODE10:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1:�������� 1��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ZERO_MODE11:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1:�������� 1��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ZERO_MODE12:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 3���������� 3��0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_ONE_MODE1:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 3���������� 3��0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ONE_MODE2:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 3���������� 3��0:��෢��һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_ONE_MODE3:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 3���������� 3��0:��෢��һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ONE_MODE4:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 3���������� 3��1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_ONE_MODE5:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 3���������� 3��1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ONE_MODE6:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 3���������� 3��1:���ٷ���һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_ONE_MODE7:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 3���������� 3��1:���ٷ���һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ONE_MODE8:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 3���������� 3��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ONE_MODE9:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 3���������� 3��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ONE_MODE10:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 3���������� 3��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ONE_MODE11:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 3���������� 3��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_ONE_MODE12:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 4���������� 4��0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_TWO_MODE1:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 4���������� 4��0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_TWO_MODE2:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 4���������� 4��0:��෢��һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_TWO_MODE3:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 4���������� 4��0:��෢��һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_TWO_MODE4:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 4���������� 4��1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_TWO_MODE5:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 4���������� 4��1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_TWO_MODE6:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 4���������� 4��1:���ٷ���һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_TWO_MODE7:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 4���������� 4��1:���ٷ���һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_TWO_MODE8:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 4���������� 4��2:ֻ����һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_TWO_MODE9:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 4���������� 4��2:ֻ����һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_TWO_MODE10:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 4���������� 4��2:ֻ����һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_ONENET_TWO_MODE11:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 4���������� 4��2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_ONENET_TWO_MODE12:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ٶ��� Endpoint��Ϣ
 * @param       ������
 *              data�� Endpoint ��Ϣ������ 32 �ֽڡ�
 *              Ĭ�ϲ���: AT+BAIEP="alientek"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_baiduyun_endpoint(char *data)
{
    uint8_t ret;
    char cmd[32];
    
    sprintf(cmd, "AT+BAIEP=\"%s\"", data);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ٶ��� name��Ϣ
 * @param       ������
 *              name�� name ��Ϣ������ 32 �ֽڡ�
 *              Ĭ�ϲ���: AT+BAINM="name"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_baiduyun_name(char *name)
{
    uint8_t ret;
    char cmd[32];
    
    sprintf(cmd, "AT+BAINM=\"%s\"", name);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ٶ��� key��Ϣ
 * @param       ������
 *              key�� key ��Ϣ������ 32 �ֽڡ�
 *              Ĭ�ϲ���: AT+BAIKEY="key"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_baiduyun_key(char *key)
{
    uint8_t ret;
    char cmd[32];
    
    sprintf(cmd, "AT+BAIKEY=\"%s\"", key);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ٶ��Ƶ�����Ϣ
 * @param       ������
 *              region�� ������Ϣ�� ���� 32 �ֽڡ� 
 *              �ٶ�����һ������λ�ã����� ������-���ݡ� ��Ӧ��gz��,
 *              ������-��������Ӧ��bj�� ��
 *              Ĭ�ϲ���: AT+BAIRI="gz"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_baiduyun_region(char *reg)
{
    uint8_t ret;
    char cmd[32];
    
    sprintf(cmd, "AT+BAIRI=\"%s\"", reg);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ٶ��ƶ���������Ϣ
 * @param       ������
 *              sub:����������Ϣ�� ���ƴ�С 100 �ֽڡ�
 *              Ĭ�ϲ���: AT+BAISUB="atk/sub"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_baiduyun_sub_title(char *title)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+BAISUB=\"%s\"", title);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C �ٶ��Ʒ���������Ϣ
 * @param       ������
 *              pub:����������Ϣ�� ���ƴ�С 100 �ֽڡ�
 *              Ĭ�ϲ���: AT+BAIPUB="atk/spub"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_baiduyun_pub_title(char *title)
{
    uint8_t ret;
    char cmd[100];
    
    sprintf(cmd, "AT+BAIPUB=\"%s\"", title);
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}

/**
 * @brief       ATK-IDM750C ONENET������Ϣ
 * @param       ������
 *              type:�豸����
 *              0���豸��
 *              1��������
 *              qos����Ϣ����ȼ�
 *              0:��෢��һ��
 *              1:���ٷ���һ��
 *              2:ֻ����һ��
 *              retain:��������״̬
 *              0:���������ᱣ��������Ϣ
 *              1:����������������Ϣ
 *              clean_session:����Ự״̬
 *              0: �ͻ������ߺ� ����������Ự��Ϣ
 *              1: �ͻ������ߺ� �������Զ��ݻ���������
 *              keepalive:����ʱ��,��Χ�� 30-1200����λ���롣
 *              Ĭ�ϲ���: AT+BAICON="0","0","0","1","300"
 * @retval      ATK_IDM750C_EOK  : ���óɹ�
 *              ATK_IDM750C_ERROR: ����ʧ��
 */
uint8_t atk_idm750c_baiduyun_para_information(atk_idm750c_baiduyun_para_mode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        /* 0���豸�ͣ�0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE1:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������ݣ�Ĭ�ϣ� */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE2:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�0:��෢��һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE3:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�0:��෢��һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE4:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE5:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE6:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�1:���ٷ���һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE7:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�1:���ٷ���һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE8:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�2:ֻ����һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE9:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�2:ֻ����һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE10:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�2:ֻ����һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ  */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE11:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0���豸�ͣ�2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE12:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1�������ͣ�0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE1:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1�������ͣ�0:��෢��һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE2:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1�������ͣ�0:��෢��һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE3:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1�������ͣ�0:��෢��һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE4:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1�������ͣ�1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE5:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1�������ͣ�1:���ٷ���һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE6:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1�������ͣ�1:���ٷ���һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE7:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1�������ͣ�1:���ٷ���һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE8:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1�������ͣ�2:ֻ����һ�Σ�0�����������ᱣ��������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE9:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1�������ͣ�2:ֻ����һ�Σ�0�����������ᱣ��������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE10:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1�������ͣ�2:ֻ����һ�Σ�1������������������Ϣ��0���ͻ������ߺ󣬷���������Ự��Ϣ */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE11:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1�������ͣ�2:ֻ����һ�Σ�1������������������Ϣ��1���ͻ������ߺ󣬷������Զ��ݻ��������� */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE12:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        default:
        {
            return ATK_IDM750C_EINVAL;
        }
    }
    
    ret  = atk_idm750c_send_cmd_to_dtu(cmd, "OK", ATK_IDM750C_TIME);
    
    if (ret == ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_EOK;
    }
    else
    {
        return ATK_IDM750C_ERROR;
    }
}
