#ifndef __ATK_IDM750C_UART_H
#define __ATK_IDM750C_UART_H

#include "stm32f1xx.h"

/* 引脚定义 */
#define ATK_IDM750C_UART_TX_GPIO_PORT         GPIOC
#define ATK_IDM750C_UART_TX_GPIO_PIN          GPIO_PIN_12
#define ATK_IDM750C_UART_TX_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)

#define ATK_IDM750C_UART_RX_GPIO_PORT         GPIOD
#define ATK_IDM750C_UART_RX_GPIO_PIN          GPIO_PIN_2
#define ATK_IDM750C_UART_RX_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)

#define ATK_IDM750C_UART_INTERFACE            UART5
#define ATK_IDM750C_UART_IRQn                 UART5_IRQn
#define ATK_IDM750C_UART_IRQHandler           UART5_IRQHandler
#define ATK_IDM750C_UART_CLK_ENABLE()         do{ __HAL_RCC_UART5_CLK_ENABLE(); }while(0)

/* UART收发缓冲大小 */
#define ATK_IDM750C_UART_RX_BUF_SIZE          1024
#define ATK_IDM750C_UART_TX_BUF_SIZE          512

/* 操作函数 */
void atk_idm750c_uart_printf(char *fmt, ...);           /* ATK-IDM750C UART printf */
void atk_idm750c_uart_rx_restart(void);                 /* ATK-IDM750C UART重新开始接收数据 */
uint8_t *atk_idm750c_uart_rx_get_frame(void);           /* 获取ATK-IDM750C UART接收到的一帧数据 */
uint8_t *atk_idm750c_uart_rx_get_frame1(void);
uint16_t atk_idm750c_uart_rx_get_frame_len(void);       /* 获取ATK-IDM750C UART接收到的一帧数据的长度 */
void atk_idm750c_uart_init(uint32_t baudrate);          /* ATK-IDM750C UART初始化 */
extern uint8_t g_uart_tx_buf[ATK_IDM750C_UART_TX_BUF_SIZE]; /* ATK-IDM750C UART发送缓冲 */

#endif
