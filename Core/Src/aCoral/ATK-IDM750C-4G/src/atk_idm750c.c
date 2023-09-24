#include "atk_idm750c_cfg.h"
#include "atk_idm750c_uart.h"
#include "atk_idm750c.h"
#include "string.h"

static uint8_t dtu_rxcmdbuf[DTU_RX_CMD_BUF_SIZE];       /*处理DTU相关数据缓存*/

/**
 * @brief       ATK-IDM750C模块初始化
 * @param       baudrate: ATK-IDM750C UART通讯波特率
 * @retval      ATK_IDM750C_EOK  : ATK-IDM750C初始化成功，函数执行成功
 *              ATK_IDM750C_ERROR: ATK-IDM750C初始化失败，函数执行失败
 */
uint8_t atk_idm750c_init(uint32_t baudrate)
{
    atk_idm750c_uart_init(baudrate);
    atk_idm750c_dtu_enter_configmode();
    if (atk_idm750c_at_test() != ATK_IDM750C_EOK)   /* ATK-IDM750C AT指令测试 */
    {
        return ATK_IDM750C_ERROR;
    }
    
    return ATK_IDM750C_EOK;
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理+ATK ERROR信息
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * @return      无
 */
static void dtu_urc_atk_error(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_atk_error\r\n");
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理Please check SIM Card信息
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * @return      无
 */
static void dtu_urc_error_sim(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_error_sim\r\n");
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理Please check GPRS信息
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * @return      无
 */
static void dtu_urc_error_gprs(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_error_gprs\r\n");
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理Please check CSQ信息
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * @return      无
 */
static void dtu_urc_error_csq(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_error_csq\r\n");
}

/**
 * @brief       DTU自动上报URC信息处理函数:处理Please check MQTT Parameter信息
 * @param       data    :   接收到DTU的URC数据缓存
 * @param       len     :   URC数据长度
 * @return      无
 */
static void dtu_urc_error_mqtt(const char *data, uint32_t len)
{
    acoral_print("\r\nURC :   dtu_urc_error_mqtt\r\n");
}

static _dtu_urc_st DTU_ATK_M750_URC[ATK_IDM750C_DTU_URC_SIZE] =
{
    {"+ATK ERROR:",                         dtu_urc_atk_error},         /*DTU存在问题，需要联系技术支持进行确认*/
    {"Please check SIM Card !!!\r\n",       dtu_urc_error_sim},         /*DTU未检测到手机卡,请检查手机卡是否正确插入*/
    {"Please check GPRS !!!\r\n",           dtu_urc_error_gprs},        /*请检查SIM卡是否欠费*/
    {"Please check CSQ !!!\r\n",            dtu_urc_error_csq},         /*请检查天线是否正确接入，并确保天线位置的正确性*/
    {"Please check MQTT Parameter !!!\r\n", dtu_urc_error_mqtt},        /*MQTT参数有误*/
};

/**
 * @brief       处理DTU主动上报的URC信息数据，注意：串口每接收一个字节数据，都需要通过函数入口传输进来
 * @param       ch:串口接收的一个字节数据
 * @return      无
 */
void atk_idm750c_dtu_get_urc_info(uint8_t ch)
{
    static uint8_t ch_last = 0;
    static uint32_t rx_len = 0;
    int i;
    
    /*接收DTU数据*/
    dtu_rxcmdbuf[rx_len++] = ch;
    
    if (rx_len >= DTU_RX_CMD_BUF_SIZE)
    {
        /*超长处理*/
        ch_last = 0;
        rx_len = 0;
        memset(dtu_rxcmdbuf, 0, DTU_RX_CMD_BUF_SIZE);
    }
    
    /*处理DTU的URC数据*/
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
 * @brief       ATK-IDM750C发送AT指令
 * @param       cmd: 待发送的AT指令
 *              ack: 等待的响应
 *              timeout: 等待超时时间
 * @retval      ATK_IDM750C_EOK: 函数执行成功
 *              ATK_IDM750C_ETIMEOUT: 等待期望应答超时，函数执行失败
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
 * @brief       ATK-IDM750C AT指令测试
 * @param       无
 * @retval      ATK_IDM750C_EOK: AT指令测试成功
 *              ATK_IDM750C_ERROR: AT指令测试失败
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
 * @brief       DTU进入配置状态
 * 
 * @param       无
 * 
 * @return      0  :    成功进入配置状态
 *             -1  :    进入配置状态失败
 */
uint8_t atk_idm750c_dtu_enter_configmode(void)
{
    uint8_t ret;
    
    /* 发送+++准备进入配置状态 */
    atk_idm750c_uart_rx_restart();
    atk_idm750c_uart_printf("+++");
    ret = atk_idm750c_send_cmd_to_dtu(NULL, "atk", 1000);
    if (ret != ATK_IDM750C_EOK)
    {
        return ATK_IDM750C_ERROR;
    }
    
    /* 发送atk确认进入配置状态 */
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
 * @brief       ATK-IDM750C恢复出厂设置
 * @param       无
 * @retval      ATK_IDM750C_EOK  : 恢复出场设置成功
 *              ATK_IDM750C_ERROR: 恢复出场设置失败
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
 * @brief       ATK-IDM750C查询版本号
 * @param       无
 * @retval      ATK_IDM750C_EOK  : 查询版本号成功
 *              ATK_IDM750C_ERROR: 查询版本号失败
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
 * @brief       ATK-IDM750C查询IMEI码
 * @param       无
 * @retval      ATK_IDM750C_EOK  : 查询IMEI码成功
 *              ATK_IDM750C_ERROR: 查询IMEI码失败
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
 * @brief       ATK-IDM750C查询ICCID码
 * @param       无
 * @retval      ATK_IDM750C_EOK  : 查询ICCID码成功
 *              ATK_IDM750C_ERROR: 查询ICCID码失败
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
 * @brief       ATK-IDM750C查询IMSI码
 * @param       无
 * @retval      ATK_IDM750C_EOK  : 查询IMSI码成功
 *              ATK_IDM750C_ERROR: 查询IMSI码失败
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
 * @brief       ATK-IDM750C查询产品序列号
 * @param       无
 * @retval      ATK_IDM750C_EOK  : 查询产品序列号成功
 *              ATK_IDM750C_ERROR: 查询产品序列号失败
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
 * @brief       ATK-IDM750C进入透传状态
 * @param       无
 * @retval      ATK_IDM750C_EOK  : 进入透传状态成功
 *              ATK_IDM750C_ERROR: 进入透传状态失败
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
 * @brief       ATK-IDM750C 打开指令回显
 * @param       enable:指令回显的使能状态
 *              ON:指令回显
 *              OFF:指令不回显
 * @retval      ATK_IDM750C_EOK  : 指令回显成功
 *              ATK_IDM750C_ERROR: 指令回显失败
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
 * @brief       ATK-IDM750C 工作模式设置
 * @param       无
 * @retval      ATK_IDM750C_EOK  : 工作模式设置测试成功
 *              ATK_IDM750C_ERROR: 工作模式设置测试失败
 */
uint8_t atk_idm750c_query_workmode(atk_idm750c_workmode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        case ATK_IDM750C_WORKMODE_NET:       /* 网络透传模式 */
        {
            sprintf(cmd, "AT+WORK=\"NET\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_HTTP:      /* HTTP透传模式 */
        {
            sprintf(cmd, "AT+WORK=\"HTTP\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_MQTT:      /* MQTT透传模式 */
        {
            sprintf(cmd, "AT+WORK=\"MQTT\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_ALIYUN:    /* 阿里云透传模式 */
        {
            sprintf(cmd, "AT+WORK=\"ALIYUN\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_ONENET:    /* OneNET透传模式 */
        {
            sprintf(cmd, "AT+WORK=\"ONENET\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_BAUDUYUN:  /* 百度云透传模式 */
        {
            sprintf(cmd, "AT+WORK=\"BAIDUYUN\"");
            break;
        }
        case ATK_IDM750C_WORKMODE_RNDIS:     /* 无线上网工作模式 */
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
 * @brief       ATK-IDM750C 串口参数
 * @param       boud:串口波特率
 *              stop:串口停止位
 *              data：串口数据位
 *              parity:串口奇偶校验位
 * @retval      ATK_IDM750C_EOK  : 串口参数配置测试成功
 *              ATK_IDM750C_ERROR: 串口参数配置测试失败
 */
uint8_t atk_idm750c_uart_parameter(atk_idm750c_uartmode_t mode)
{
    uint8_t ret;
    char cmd[64];
    char *NONE;
    
    switch (mode)
    {
        case ATK_IDM750C_UARTMODE_FIRST:      /* boud：1200，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",1200,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_SECOND:     /* boud：2400，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",2400,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_THIRD:      /* boud：4800，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",4800,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_FOURTH:     /* boud：9600，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",9600,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_FIFTH:      /* boud：14400，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",14400,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_SIXTH:      /* boud：19200，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",19200,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_SEVENTH:    /* boud：38400，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",38400,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_EIGHTH:     /* boud：57600，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",57600,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_NINTH:      /* boud：115200，stop：1，data：8，parity：NONE（默认） */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",115200,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_TENTH:      /* boud：128000，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",128000,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_ELEVENTH:   /* boud：230400，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",230400,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_TWELFTH:    /* boud：460800，stop：1，data：8，parity：NONE */
        {
            sprintf(cmd, "AT+UART=\"%d\",\"%d\",\"%d\",\"%s\"",460800,1,8,NONE);
            break;
        }
        case ATK_IDM750C_UARTMODE_THIRTEENTH: /* boud：921600，stop：1，data：8，parity：NONE */
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
 * @brief       ATK-IDM750C 串口透传配置
 * @param       enable:串口透传指令的使能状态
 *              ON:使用串口透传指令
 *              OFF:不使用串口透传指令
 * @retval      ATK_IDM750C_EOK  : 串口透传配置成功
 *              ATK_IDM750C_ERROR: 串口透传配置失败
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
 * @brief       ATK-IDM750C串口打包参数
 * @param       length：串口打包长度， 范围： 1-1024(字节)
 *              time:串口打包时间,范围： 50ms-10000ms
 * @retval      ATK_IDM750C_EOK  : 进入透传状态成功
 *              ATK_IDM750C_ERROR: 进入透传状态失败
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
 * @brief       ATK-IDM750C 网络透传指令
 * @param       enable:网络透传指令的使能状态
 *              ON:使用网络透传指令
 *              OFF:不使用网络透传指令
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 透传指令密钥
 * @param       data:指令密钥数据，最大限制 20 个字节
 *              data 默认参数： ALIENTEK
 * @retval      ATK_IDM750C_EOK  : 透传指令密钥配置成功
 *              ATK_IDM750C_ERROR: 透传指令密钥配置失败
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
 * @brief       ATK-IDM750C 用户自定义数据
 * @param       data:用户自定义数据，最大限制 32 个字节
 *              data 默认参数： ALIENTEK
 * @retval      ATK_IDM750C_EOK  : 用户自定义数据配置成功
 *              ATK_IDM750C_ERROR: 用户自定义数据配置失败
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
 * @brief       ATK-IDM750C 无网络数据传输自动重启时间
 * @param       time:自动重启时间,时间设置范围： 60s-3600s
 *              time 默认参数： 1200
 * @retval      ATK_IDM750C_EOK  : 无网络数据传输自动重启时间配置成功
 *              ATK_IDM750C_ERROR: 无网络数据传输自动重启时间配置失败
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
 * @brief       ATK-IDM750C 网络连接超时重启时间
 * @param       time:超时重启时间，范围： 30s-6000s
 *              time 默认参数： 120s
 * @retval      ATK_IDM750C_EOK  : 网络连接超时重启时间配置成功
 *              ATK_IDM750C_ERROR: 网络连接超时重启时间配置失败
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
 * @brief       ATK-IDM750C 发送英文短信
 * @param       phone:目标电话号码，最大限制 20 个字节
 *              data： 短信内容，最大限制 160 个字节
 * @retval      ATK_IDM750C_EOK  : 发送英文短信成功
 *              ATK_IDM750C_ERROR: 发送英文短信失败
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
 * @brief       ATK-IDM750C 第一路连接参数
 * @param       TCP:TCP 模式
 *              UDP:UDP 模式
 *              ip： 服务器地址（IP 地址和域名都支持） ,最大限制 100 个字节
 *              port：服务器端口号，大小范围 0-65535
 *              第一路连接默认参数： AT+LINK1="TCP","cloud.alientek.com","59666"
 * @retval      ATK_IDM750C_EOK  : 第一路连接参数配置成功
 *              ATK_IDM750C_ERROR: 第一路连接参数配置失败
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
 * @brief       ATK-IDM750C 第二路连接参数
 * @param       TCP:TCP 模式
 *              UDP:UDP 模式
 *              ip： 服务器地址（IP 地址和域名都支持） ,最大限制 100 个字节
 *              port：服务器端口号，大小范围 0-65535
 *              第二路连接默认参数： AT+LINK1="TCP","cloud.alientek.com","59666"
 * @retval      ATK_IDM750C_EOK  : 第二路连接参数配置成功
 *              ATK_IDM750C_ERROR: 第二路连接参数配置失败
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
 * @brief       ATK-IDM750C 第三路连接参数
 * @param       TCP:TCP 模式
 *              UDP:UDP 模式
 *              ip： 服务器地址（IP 地址和域名都支持） ,最大限制 100 个字节
 *              port：服务器端口号，大小范围 0-65535
 *              第三路连接默认参数： AT+LINK1="TCP","cloud.alientek.com","59666"
 * @retval      ATK_IDM750C_EOK  : 第三路连接参数配置成功
 *              ATK_IDM750C_ERROR: 第三路连接参数配置失败
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
 * @brief       ATK-IDM750C 第四路连接参数
 * @param       TCP:TCP 模式
 *              UDP:UDP 模式
 *              ip： 服务器地址（IP 地址和域名都支持） ,最大限制 100 个字节 
 *              port：服务器端口号，大小范围 0-65535
 *              第四路连接默认参数： AT+LINK1="TCP","cloud.alientek.com","59666"
 * @retval      ATK_IDM750C_EOK  : 第四路连接参数配置成功
 *              ATK_IDM750C_ERROR: 第四路连接参数配置失败
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
 * @brief       ATK-IDM750C 第一路连接使能状态
 * @param       enable:连接使能状态
 *              ON:启用连接
 *              OFF:不启用连接
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 第二路连接使能状态
 * @param       enable:连接使能状态
 *              ON:启用连接
 *              OFF:不启用连接
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 第三路连接使能状态
 * @param       enable:连接使能状态
 *              ON:启用连接
 *              OFF:不启用连接
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 第四路连接使能状态
 * @param       enable:连接使能状态
 *              ON:启用连接
 *              OFF:不启用连接
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 第一路连接模式
 * @param       mode:连接模式
 *              LONG:长连接（默认）
 *              SHORT:短连接
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 第二路连接模式
 * @param       mode:连接模式
 *              LONG:长连接（默认）
 *              SHORT:短连接
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 第三路连接模式
 * @param       mode:连接模式
 *              LONG:长连接（默认）
 *              SHORT:短连接
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 第四路连接模式
 * @param       mode:连接模式
 *              LONG:长连接（默认）
 *              SHORT:短连接
 * @retval      ATK_IDM750C_EOK  : 网络透传配置成功
 *              ATK_IDM750C_ERROR: 网络透传配置失败
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
 * @brief       ATK-IDM750C 查询第一路连接状态
 * @param       state：网络连接状态
 *              CONNECTED：已连接
 *              DISCONNECTED：未连接
 * @retval      ATK_IDM750C_EOK  : 查询成功
 *              ATK_IDM750C_ERROR: 查询失败
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
 * @brief       ATK-IDM750C 查询第二路连接状态
 * @param       state：网络连接状态
 *              CONNECTED：已连接
 *              DISCONNECTED：未连接
 * @retval      ATK_IDM750C_EOK  : 查询成功
 *              ATK_IDM750C_ERROR: 查询失败
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
 * @brief       ATK-IDM750C 查询第三路连接状态
 * @param       state：网络连接状态
 *              CONNECTED：已连接
 *              DISCONNECTED：未连接
 * @retval      ATK_IDM750C_EOK  : 查询成功
 *              ATK_IDM750C_ERROR: 查询失败
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
 * @brief       ATK-IDM750C 查询第四路连接状态
 * @param       state：网络连接状态
 *              CONNECTED：已连接
 *              DISCONNECTED：未连接
 * @retval      ATK_IDM750C_EOK  : 查询成功
 *              ATK_IDM750C_ERROR: 查询失败
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
 * @brief       ATK-IDM750C 第一路超时重连时间
 * @param       time：超时重连时间，范围1s-60s
 *              time默认值为：5s
 * @retval      ATK_IDM750C_EOK  : 重连成功
 *              ATK_IDM750C_ERROR: 重连失败
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
 * @brief       ATK-IDM750C 第二路超时重连时间
 * @param       time：超时重连时间，范围1s-60s
 *              time默认值为：5s
 * @retval      ATK_IDM750C_EOK  : 重连成功
 *              ATK_IDM750C_ERROR: 重连失败
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
 * @brief       ATK-IDM750C 第三路超时重连时间
 * @param       time：超时重连时间，范围1s-60s
 *              time默认值为：5s
 * @retval      ATK_IDM750C_EOK  : 重连成功
 *              ATK_IDM750C_ERROR: 重连失败
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
 * @brief       ATK-IDM750C 第四路超时重连时间
 * @param       time：超时重连时间，范围1s-60s
 *              time默认值为：5s
 * @retval      ATK_IDM750C_EOK  : 重连成功
 *              ATK_IDM750C_ERROR: 重连失败
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
 * @brief       ATK-IDM750C 第一路短连接持续时间
 * @param       time：连接超时时间，范围1s-100s
 *              time默认值为：10s
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 第二路短连接持续时间
 * @param       time：连接超时时间，范围1s-100s
 *              time默认值为：10s
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 第三路短连接持续时间
 * @param       time：连接超时时间，范围1s-100s
 *              time默认值为：10s
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 第四路短连接持续时间
 * @param       time：连接超时时间，范围1s-100s
 *              time默认值为：10s
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C APN配置
 * @param       apn:APN类型，最大限制98个字节
 *              以下为通用接入点：
 *              自动获取APN接入点：AUTO，中国移动接入点：CMNET
 *              中国联通接入点：UNINET，中国电信接入点：CTNET
 *              user name：用户名，若没有便不需要填写，最大限制30个字节
 *              password：密码，若没有便不需要填写，最大限制30个字节
 *              CONNECTED：已连接
 *              DISCONNECTED：未连接
 * @retval      ATK_IDM750C_EOK  : 发送英文短信成功
 *              ATK_IDM750C_ERROR: 发送英文短信失败
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
 * @brief       ATK-IDM750C 指定数据发送到一路连接
 * @param       socket：发送数据到哪一路连接，参数范围：1-4
 *              data：需要发送的数据
 *              ALIENTEK：透传指令密钥（可以通过 AT 指令或者上位机修改，出厂默认为： ALIENTEK）
 *              1：为第一路连接
 *              alientek：透传数据（逗号后面的数据全部为透传数据）
 * @retval      ATK_IDM750C_EOK  : 发送成功
 *              ATK_IDM750C_ERROR: 发送失败
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
 * @brief       ATK-IDM750C 网络透传模式数据发送模式
 * @param       enable 数据发送模式:
 *              ON : 快发模式
 *              OFF：慢发模式
 *              默认为快发模式： +SENDFAST: "ON"
 * @retval      ATK_IDM750C_EOK  : 发送成功
 *              ATK_IDM750C_ERROR: 发送失败
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
 * @brief       ATK-IDM750C 原子云相关指令
 * @param       enable 启用状态
 *              ON : 启用
 *              OFF：不启用
 *              默认参数：OFF
 * @retval      ATK_IDM750C_EOK  : 启用成功
 *              ATK_IDM750C_ERROR: 启用失败
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
 * @brief       ATK-IDM750C 原子云设备编号
 * @param       data： 设备编号，固定为20个字节，且为纯数字
 *              data默认参数：12345678901234567890，具体参数
 *              根据原子云生成的设备编号为准
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 原子云设备密码
 * @param       data：设备密码，固定8个字节，且为纯数字
 *              默认参数：12345678,用户也可以自行设置
 * @retval      ATK_IDM750C_EOK  : 发送成功
 *              ATK_IDM750C_ERROR: 发送失败
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
 * @brief       ATK-IDM750C 注册包启用状态
 * @param       enable 启用状态
 *              ON : 启用
 *              OFF：不启用
 *              默认参数：OFF
 * @retval      ATK_IDM750C_EOK  : 发送成功
 *              ATK_IDM750C_ERROR: 发送失败
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
 * @brief       ATK-IDM750C 注册包数据
 * @param       data:注册包数据， 最大限制 100 个字节，
 *              且输入十六进制字符串必须为偶数！ 不足补零。
 *              data 默认参数： 414C49454E54454B2D5245474454
 * @retval      ATK_IDM750C_EOK  : 发送成功
 *              ATK_IDM750C_ERROR: 发送失败
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
 * @brief       ATK-IDM750C 注册包发送方式
 * @param       mode:注册包发送模式
 *              LINK:成功连接建立后发送一次
 *              DATA:每帧数据前发送发送一次
 *              ALL：同时支持以上两种发送模式
 *              mode 默认参数： LINK
 * @retval      ATK_IDM750C_EOK  : 发送成功
 *              ATK_IDM750C_ERROR: 发送失败
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
 * @brief       ATK-IDM750C 注册包数据类型
 * @param       type:注册包数据类型
                IMEI:把 IMEI 当做注册包数据
                ICCID:把 ICCID 当做注册包数据
                IMSI:把 IMSI 当做注册包数据
                USER： 用户自定义注册包数据，通过 AT+REGDT 指令配置
                type 默认参数： IMEI
 * @retval      ATK_IDM750C_EOK  : 发送成功
 *              ATK_IDM750C_ERROR: 发送失败
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
 * @brief       ATK-IDM750C 心跳包启用状态
 * @param       enable 启用状态
 *              ON : 启用
 *              OFF：不启用
 *              默认参数：OFF
 * @retval      ATK_IDM750C_EOK  : 开启成功
 *              ATK_IDM750C_ERROR: 开启失败
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
 * @brief       ATK-IDM750C 心跳包数据
 * @param       data:心跳包数据， 最大限制 100 个字节，
 *              且输入十六进制字符串必须为偶数！ 不足补零。
 *              data 默认参数： 414C49454E54454B2D4852544454
 * @retval      ATK_IDM750C_EOK  : 发送成功
 *              ATK_IDM750C_ERROR: 发送失败
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
 * @brief       ATK-IDM750C 心跳包时间间隔
 * @param       time:单位秒，范围： 5s-1200s
 *              time 默认参数： 120s
 * @retval      ATK_IDM750C_EOK  : 心跳包时间间隔配置成功
 *              ATK_IDM750C_ERROR: 心跳包时间间隔配置失败
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
 * @brief       ATK-IDM750C HTTP相关指令
 * @param       mode:HTTP 请求方式
 *              GET: GET 方式
 *              POST:POST 方式
 *              mode 默认参数： GET
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C HTTP URL地址
 * @param       参数：
 *              data:URL 地址，最大限制 200 个字节。
 *              data 默认参数: AT+HTTPURL=" https://cloud.alientek.com/testfordtu?data="
 * @retval      ATK_IDM750C_EOK  : 地址配置成功
 *              ATK_IDM750C_ERROR: 地址配置失败
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
 * @brief       ATK-IDM750C HTTP请求超时时间
 * @param       time:请求超时时间，范围： 5s-60s
 *              time 默认参数： 10s
 * @retval      ATK_IDM750C_EOK  : HTTP请求超时时间隔配置成功
 *              ATK_IDM750C_ERROR: HTTP请求超时时间隔配置失败
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
 * @brief       ATK-IDM750C HTTP请求头信息
 * @param       header:请求头信息， 限制大小 200 字节。
 *              header 默认参数: AT+HTTPHD="Connection:close"
 * @retval      ATK_IDM750C_EOK  : HTTP请求头信息配置成功
 *              ATK_IDM750C_ERROR: HTTP请求头信息配置失败
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
 * @brief       ATK-IDM750C MQTT clientid 信息
 * @param       参数：
 *              clientid:MQTT 的 clientid 信息，限制 200 个字节。
 *              clientid 默认参数: AT+MQTTCD="alientek"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
 */
uint8_t atk_idm750c_mqtt_clientid(char *clientid)
{
    uint8_t ret;
    char cmd[128];
    
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
 * @brief       ATK-IDM750C MQTT username 信息
 * @param       参数：
 *              clientid:MQTT 的 username 信息，限制 200 个字节。
 *              clientid 默认参数: AT+MQTTUN="admin"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C MQTT password 信息
 * @param       参数：
 *              clientid:MQTT 的 password 信息，限制 200 个字节。
 *              clientid 默认参数: AT+MQTTUN="password"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C MQTT 服务器信息
 * @param       参数：
 *              ip:MQTT 服务器地址，限制 128 个字节。 可以是 IP 地址也可以为域名
 *              port:MQTT 服务器端口号， 限制 0-65535。
 *              默认参数: AT+MQTTIP="cloud.alientek.com","1883"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C MQTT 订阅主题信息
 * @param       参数：
 *              sub:订阅主题信息， 限制大小 100 字节。
 *              默认参数: AT+MQTTSUB="atk/sub"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C MQTT 发布主题信息
 * @param       参数：
 *              pub:订阅主题信息， 限制大小 100 字节。
 *              默认参数: AT+MQTTPUB="atk/pub"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C MQTT 参数信息
 * @param       参数：
 *              qos：消息服务等级
 *              0:最多发送一次
 *              1:至少发送一次
 *              2:只发送一次
 *              retain:遗嘱保留状态
 *              0:服务器不会保存遗嘱信息
 *              1:服务器保存遗嘱信息
 *              clean_session:清除会话状态
 *              0: 客户端离线后， 服务器保存会话消息
 *              1: 客户端离线后， 服务器自动摧毁所有数据
 *              keepalive:保活时间,范围： 1-65535， 单位：秒。
 *              默认参数: AT+MQTTCON="0","0","1","300"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
 */
uint8_t atk_idm750c_mqtt_para_information(atk_idm750c_mqtt_para_mode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        /* 0：最多发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_MQTT_ZERO_MODE1:
        {
            sprintf(cmd, "AT+MQTTCON=\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0：最多发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 (默认)*/
        case ATK_IDM750C_MQTT_ZERO_MODE2:
        {
            sprintf(cmd, "AT+MQTTCON=\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0：最多发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_MQTT_ZERO_MODE3:
        {
            sprintf(cmd, "AT+MQTTCON=\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0：最多发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_MQTT_ZERO_MODE4:
        {
            sprintf(cmd, "AT+MQTTCON=\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1：至少发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_MQTT_ONE_MODE1:
        {
            sprintf(cmd, "AT+MQTTCON=\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1：至少发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_MQTT_ONE_MODE2:
        {
            sprintf(cmd, "AT+MQTTCON=\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1：至少发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_MQTT_ONE_MODE3:
        {
            sprintf(cmd, "AT+MQTTCON=\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1：至少发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_MQTT_ONE_MODE4:
        {
            sprintf(cmd, "AT+MQTTCON=\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 2：只发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_MQTT_TWO_MODE1:
        {
            sprintf(cmd, "AT+MQTTCON=\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 2：只发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_MQTT_TWO_MODE2:
        {
            sprintf(cmd, "AT+MQTTCON=\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 2：只发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_MQTT_TWO_MODE3:
        {
            sprintf(cmd, "AT+MQTTCON=\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 2：只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
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
 * @brief       ATK-IDM750C 阿里云ProductKey信息
 * @param       参数：
 *              pk:阿里云 ProductKey 信息， 限制大小 20 字节。
 *              默认参数: AT+ALIPK="ProductKey"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 阿里云DeviceSecret信息
 * @param       参数：
 *              ds:阿里云 DeviceSecret 信息， 限制大小 64 字节。
 *              默认参数: AT+ALIDS="DeviceSecret
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 阿里云DeviceName信息
 * @param       参数：
 *              dn: 阿里云 DeviceName 信息， 限制大小 32 字节。
 *              默认参数: AT+ALIDN="DeviceName"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 阿里云地域信息
 * @param       参数：
 *              ri: 阿里云地域信息， 限制大小 20 字节。
 *              默认参数: AT+ALIRI="cn-shanghai
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 阿里云订阅主题信息
 * @param       参数：
 *              sub:订阅主题信息， 限制大小 100 字节。
 *              默认参数: AT+ALISUB="get"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 阿里云发布主题信息
 * @param       参数：
 *              pub:订阅主题信息， 限制大小 100 字节。
 *              默认参数: AT+ALIPUB="update"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 阿里云参数信息
 * @param       参数：
 *              qos：消息服务等级
 *              0:最多发送一次
 *              1:至少发送一次
 *              2:只发送一次
 *              retain:遗嘱保留状态
 *              0:服务器不会保存遗嘱信息
 *              1:服务器保存遗嘱信息
 *              clean_session:清除会话状态
 *              0: 客户端离线后， 服务器保存会话消息
 *              1: 客户端离线后， 服务器自动摧毁所有数据
 *              keepalive:保活时间,范围： 30-1200，单位：秒。 。
 *              默认参数: AT+ALICON="0","0","1","300"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
 */
uint8_t atk_idm750c_aliyun_para_information(atk_idm750c_aliyun_para_mode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        /* 0：最多发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ALIYUN_ZERO_MODE1:
        {
            sprintf(cmd, "AT+ALICON=\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0：最多发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 （默认）*/
        case ATK_IDM750C_ALIYUN_ZERO_MODE2:
        {
            sprintf(cmd, "AT+ALICON=\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0：最多发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ALIYUN_ZERO_MODE3:
        {
            sprintf(cmd, "AT+ALICON=\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0：最多发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ALIYUN_ZERO_MODE4:
        {
            sprintf(cmd, "AT+ALICON=\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1：至少发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ALIYUN_ONE_MODE1:
        {
            sprintf(cmd, "AT+ALICON=\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1：至少发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ALIYUN_ONE_MODE2:
        {
            sprintf(cmd, "AT+ALICON=\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1：至少发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ALIYUN_ONE_MODE3:
        {
            sprintf(cmd, "AT+ALICON=\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1：至少发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ALIYUN_ONE_MODE4:
        {
            sprintf(cmd, "AT+ALICON=\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 2：只发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ALIYUN_TWO_MODE1:
        {
            sprintf(cmd, "AT+ALICON=\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 2：只发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ALIYUN_TWO_MODE2:
        {
            sprintf(cmd, "AT+ALICON=\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 2：只发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ALIYUN_TWO_MODE3:
        {
            sprintf(cmd, "AT+ALICON=\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 2：只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
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
 * @brief       ATK-IDM750C OneNET 设备ID信息
 * @param       参数：
 *              di： 设备 ID 信息，限制 20 字节。
 *              默认参数: AT+ONEDI="12345"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C OneNET 产品ID信息
 * @param       参数：
 *              pi： 产品 ID 信息，限制 20 字节。
 *              默认参数: AT+ONEPI="1234567890"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C OneNET 鉴权信息
 * @param       参数：
 *              ai： 鉴权信息信息，限制 64 字节。
 *              默认参数: AT+ONEAI="12345678901234567890"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C OneNET 服务器信息
 * @param       参数：
 *              ip： 服务器地址， 限制 40 字节。
 *              port： 服务器端口号， 范围： 0-65535
 *              默认参数: AT+ONEIP="mqtt.heclouds.com","6002"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C ONENET参数信息
 * @param       参数：
 *              data_type:数据类型（注： 请参考 OneNET 设备端接入协议-MQTT 文档中的数据类型定义）
 *              1： 数据类型 1
 *              3： 数据类型 3
 *              4： 数据类型 4
 *              qos：消息服务等级
 *              0:最多发送一次
 *              1:至少发送一次
 *              2:只发送一次
 *              retain:遗嘱保留状态
 *              0:服务器不会保存遗嘱信息
 *              1:服务器保存遗嘱信息
 *              clean_session:清除会话状态
 *              0:客户端离线后， 服务器保存会话消息
 *              1:客户端离线后，服务器自动摧毁所有数据
 *              keepalive:保活时间,范围： 120-1800，单位：秒。
 *              默认参数: AT+ONECON="1","0","0","1","300"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
 */
uint8_t atk_idm750c_onenet_para_information(atk_idm750c_onenet_para_mode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        /* 1:数据类型 1，0:最多发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_ZERO_MODE1:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1:数据类型 1，0:最多发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据（默认） */
        case ATK_IDM750C_ONENET_ZERO_MODE2:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1:数据类型 1，0:最多发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_ZERO_MODE3:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1:数据类型 1，0:最多发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ZERO_MODE4:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1:数据类型 1，1:至少发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_ZERO_MODE5:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1:数据类型 1，1:至少发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ZERO_MODE6:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1:数据类型 1，1:至少发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_ZERO_MODE7:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1:数据类型 1，1:至少发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ZERO_MODE8:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1:数据类型 1，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ZERO_MODE9:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1:数据类型 1，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ZERO_MODE10:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1:数据类型 1，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ZERO_MODE11:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1:数据类型 1，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ZERO_MODE12:
        {
            sprintf(cmd, "AT+ONECON=\"1\",\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 3：数据类型 3，0:最多发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_ONE_MODE1:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 3：数据类型 3，0:最多发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ONE_MODE2:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 3：数据类型 3，0:最多发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_ONE_MODE3:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 3：数据类型 3，0:最多发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ONE_MODE4:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 3：数据类型 3，1:至少发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_ONE_MODE5:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 3：数据类型 3，1:至少发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ONE_MODE6:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 3：数据类型 3，1:至少发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_ONE_MODE7:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 3：数据类型 3，1:至少发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ONE_MODE8:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 3：数据类型 3，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ONE_MODE9:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 3：数据类型 3，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ONE_MODE10:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 3：数据类型 3，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ONE_MODE11:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 3：数据类型 3，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_ONE_MODE12:
        {
            sprintf(cmd, "AT+ONECON=\"3\",\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 4：数据类型 4，0:最多发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_TWO_MODE1:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 4：数据类型 4，0:最多发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_TWO_MODE2:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 4：数据类型 4，0:最多发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_TWO_MODE3:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 4：数据类型 4，0:最多发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_TWO_MODE4:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 4：数据类型 4，1:至少发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_TWO_MODE5:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 4：数据类型 4，1:至少发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_TWO_MODE6:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 4：数据类型 4，1:至少发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_TWO_MODE7:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 4：数据类型 4，1:至少发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_TWO_MODE8:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 4：数据类型 4，2:只发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_TWO_MODE9:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 4：数据类型 4，2:只发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_ONENET_TWO_MODE10:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 4：数据类型 4，2:只发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_ONENET_TWO_MODE11:
        {
            sprintf(cmd, "AT+ONECON=\"4\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 4：数据类型 4，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
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
 * @brief       ATK-IDM750C 百度云 Endpoint信息
 * @param       参数：
 *              data： Endpoint 信息，限制 32 字节。
 *              默认参数: AT+BAIEP="alientek"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 百度云 name信息
 * @param       参数：
 *              name： name 信息，限制 32 字节。
 *              默认参数: AT+BAINM="name"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 百度云 key信息
 * @param       参数：
 *              key： key 信息，限制 32 字节。
 *              默认参数: AT+BAIKEY="key"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 百度云地域信息
 * @param       参数：
 *              region： 地域信息， 限制 32 字节。 
 *              百度云有一个地域位置，例如 “华南-广州” 对应“gz”,
 *              “华北-北京”对应“bj” 。
 *              默认参数: AT+BAIRI="gz"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 百度云订阅主题信息
 * @param       参数：
 *              sub:订阅主题信息， 限制大小 100 字节。
 *              默认参数: AT+BAISUB="atk/sub"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C 百度云发布主题信息
 * @param       参数：
 *              pub:发布主题信息， 限制大小 100 字节。
 *              默认参数: AT+BAIPUB="atk/spub"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
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
 * @brief       ATK-IDM750C ONENET参数信息
 * @param       参数：
 *              type:设备类型
 *              0：设备型
 *              1：数据型
 *              qos：消息服务等级
 *              0:最多发送一次
 *              1:至少发送一次
 *              2:只发送一次
 *              retain:遗嘱保留状态
 *              0:服务器不会保存遗嘱信息
 *              1:服务器保存遗嘱信息
 *              clean_session:清除会话状态
 *              0: 客户端离线后， 服务器保存会话消息
 *              1: 客户端离线后， 服务器自动摧毁所有数据
 *              keepalive:保活时间,范围： 30-1200，单位：秒。
 *              默认参数: AT+BAICON="0","0","0","1","300"
 * @retval      ATK_IDM750C_EOK  : 配置成功
 *              ATK_IDM750C_ERROR: 配置失败
 */
uint8_t atk_idm750c_baiduyun_para_information(atk_idm750c_baiduyun_para_mode_t mode)
{
    uint8_t ret;
    char cmd[30];
    
    switch (mode)
    {
        /* 0：设备型，0:最多发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE1:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0：设备型，0:最多发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据（默认） */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE2:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0：设备型，0:最多发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE3:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0：设备型，0:最多发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE4:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 0：设备型，1:至少发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE5:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0：设备型，1:至少发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE6:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0：设备型，1:至少发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE7:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0：设备型，1:至少发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE8:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 0：设备型，2:只发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE9:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 0：设备型，2:只发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE10:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 0：设备型，2:只发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息  */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE11:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 0：设备型，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ZERO_MODE12:
        {
            sprintf(cmd, "AT+BAICON=\"0\",\"2\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1：数据型，0:最多发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE1:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"0\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1：数据型，0:最多发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE2:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"0\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1：数据型，0:最多发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE3:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"0\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1：数据型，0:最多发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE4:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"0\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1：数据型，1:至少发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE5:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"1\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1：数据型，1:至少发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE6:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"1\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1：数据型，1:至少发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE7:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"1\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1：数据型，1:至少发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE8:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"1\",\"1\",\"1\",\"300\"");
            break;
        }
        /* 1：数据型，2:只发送一次，0：服务器不会保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE9:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"2\",\"0\",\"0\",\"300\"");
            break;
        }
        /* 1：数据型，2:只发送一次，0：服务器不会保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE10:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"2\",\"0\",\"1\",\"300\"");
            break;
        }
        /* 1：数据型，2:只发送一次，1：服务器保存遗嘱信息，0：客户端离线后，服务器保存会话消息 */
        case ATK_IDM750C_BAIDUYUN_ONE_MODE11:
        {
            sprintf(cmd, "AT+BAICON=\"1\",\"2\",\"1\",\"0\",\"300\"");
            break;
        }
        /* 1：数据型，2:只发送一次，1：服务器保存遗嘱信息，1：客户端离线后，服务器自动摧毁所有数据 */
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
