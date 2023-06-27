#include "atk_idm750c_uart.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef g_uart_handle;                    /* ATK-IDM750C UART */
static struct
{
    uint8_t buf[ATK_IDM750C_UART_RX_BUF_SIZE];              /* ֡���ջ��� */
    struct
    {
        uint16_t len    : 15;                               /* ֡���ճ��ȣ�sta[14:0] */
        uint16_t finsh  : 1;                                /* ֡������ɱ�־��sta[15] */
    } sta;                                                  /* ֡״̬��Ϣ */
} g_uart_rx_frame = {0};                                    /* ATK-IDM750C UART����֡������Ϣ�ṹ�� */
uint8_t g_uart_tx_buf[ATK_IDM750C_UART_TX_BUF_SIZE]; /* ATK-IDM750C UART���ͻ��� */

/**
 * @brief       ATK-IDM750C UART printf
 * @param       fmt: ����ӡ������
 * @retval      ��
 */
void atk_idm750c_uart_printf(char *fmt, ...)
{
    va_list ap;
    uint16_t len;
    
    va_start(ap, fmt);
    vsprintf((char *)g_uart_tx_buf, fmt, ap);
    va_end(ap);
    
    len = strlen((const char *)g_uart_tx_buf);
    HAL_UART_Transmit(&g_uart_handle, g_uart_tx_buf, len, HAL_MAX_DELAY);
}

/**
 * @brief       ATK-IDM750C UART���¿�ʼ��������
 * @param       ��
 * @retval      ��
 */
void atk_idm750c_uart_rx_restart(void)
{
    g_uart_rx_frame.sta.len     = 0;
    g_uart_rx_frame.sta.finsh   = 0;
}

/**
 * @brief       ��ȡATK-IDM750C UART���յ���һ֡����
 * @param       ��
 * @retval      NULL: δ���յ�һ֡����
 *              ����: ���յ���һ֡����
 */
uint8_t *atk_idm750c_uart_rx_get_frame(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
    {
        g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = '\0';
        return g_uart_rx_frame.buf;
    }
    else
    {
        return NULL;
    }
}

uint8_t *atk_idm750c_uart_rx_get_frame1(void)
{
    return g_uart_rx_frame.buf;
  
}

/**
 * @brief       ��ȡATK-IDM750C UART���յ���һ֡���ݵĳ���
 * @param       ��
 * @retval      0   : δ���յ�һ֡����
 *              ����: ���յ���һ֡���ݵĳ���
 */
uint16_t atk_idm750c_uart_rx_get_frame_len(void)
{
    if (g_uart_rx_frame.sta.finsh == 1)
    {
        return g_uart_rx_frame.sta.len;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief       ATK-IDM750C UART��ʼ��
 * @param       baudrate: UARTͨѶ������
 * @retval      ��
 */
void atk_idm750c_uart_init(uint32_t baudrate)
{
    g_uart_handle.Instance          = ATK_IDM750C_UART_INTERFACE;     /* ATK-IDM750C UART */
    g_uart_handle.Init.BaudRate     = baudrate;                     /* ������ */
    g_uart_handle.Init.WordLength   = UART_WORDLENGTH_8B;           /* ����λ */
    g_uart_handle.Init.StopBits     = UART_STOPBITS_1;              /* ֹͣλ */
    g_uart_handle.Init.Parity       = UART_PARITY_NONE;             /* У��λ */
    g_uart_handle.Init.Mode         = UART_MODE_TX_RX;              /* �շ�ģʽ */
    g_uart_handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;          /* ��Ӳ������ */
    g_uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;         /* ������ */
    HAL_UART_Init(&g_uart_handle);                                  /* ʹ��ATK-IDM750C UART
                                                                     * HAL_UART_Init()����ú���HAL_UART_MspInit()
                                                                     * �ú����������ļ�usart.c��
                                                                     */
}

/**
 * @brief       ATK-IDM750C UART�жϻص�����
 * @param       ��
 * @retval      ��
 */
void ATK_IDM750C_UART_IRQHandler(void)
{
    uint8_t tmp;
    
    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_ORE) != RESET)        /* UART���չ��ش����ж� */
    {
        __HAL_UART_CLEAR_OREFLAG(&g_uart_handle);                           /* ������չ��ش����жϱ�־ */
        (void)g_uart_handle.Instance->SR;                                   /* �ȶ�SR�Ĵ������ٶ�DR�Ĵ��� */
        (void)g_uart_handle.Instance->DR;
    }
    
    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_RXNE) != RESET)       /* UART�����ж� */
    {
        HAL_UART_Receive(&g_uart_handle, &tmp, 1, HAL_MAX_DELAY);           /* UART�������� */
        
        if (g_uart_rx_frame.sta.len < (ATK_IDM750C_UART_RX_BUF_SIZE - 1))     /* �ж�UART���ջ����Ƿ����
                                                                             * ����һλ��������'\0'
                                                                             */
        {
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;             /* �����յ�������д�뻺�� */
            g_uart_rx_frame.sta.len++;                                      /* ���½��յ������ݳ��� */
        }
        else                                                                /* UART���ջ������ */
        {
            g_uart_rx_frame.sta.len = 0;                                    /* ����֮ǰ�յ������� */
            g_uart_rx_frame.buf[g_uart_rx_frame.sta.len] = tmp;             /* �����յ�������д�뻺�� */
            g_uart_rx_frame.sta.len++;                                      /* ���½��յ������ݳ��� */
        }
    }
    
    if (__HAL_UART_GET_FLAG(&g_uart_handle, UART_FLAG_IDLE) != RESET)       /* UART���߿����ж� */
    {
        g_uart_rx_frame.sta.finsh = 1;                                      /* ���֡������� */
        
        __HAL_UART_CLEAR_IDLEFLAG(&g_uart_handle);                          /* ���UART���߿����ж� */
    }
}
