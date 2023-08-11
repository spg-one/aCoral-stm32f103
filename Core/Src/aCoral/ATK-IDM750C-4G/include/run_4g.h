#ifndef __RUN_4G_H
#define __RUN_4G_H

#include "atk_idm750c_uart.h"
#include "string.h"
#include "atk_idm750c.h"

extern uint8_t *buf_4g;//服务器消息接收缓冲区
extern uint8_t data_4g;//服务器消息标识符
void init_4g(void);
void tx_4g(void);
void rx_4g(void);

#endif
