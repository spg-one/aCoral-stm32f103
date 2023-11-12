#ifndef __RUN_4G_H
#define __RUN_4G_H

#include "atk_idm750c_uart.h"
#include "string.h"
#include "atk_idm750c.h"

typedef struct 
{
    uint8_t master_id;                  //中心站id 0x00(0000 0000)共八位，前两位表示设备类型，00表示中心站，01表示终端，后六位代表地址
    uint8_t slave_device_id;            //终端id 0x40(0100 0000) 0x41(0100 0001)
    uint8_t package_serial;             //当前升级包序号
    uint8_t data_size;                  //具体内容大小
    uint8_t data[200];                  //升级包具体内容   
} upgrade_buffer;

typedef struct 
{
    uint8_t master_id;  
    uint8_t slave_device_id; 
    uint8_t package_serial;             //请求升级包序号                
}upgrade_ack;



extern uint8_t *buf_4g;//服务器消息接收缓冲区
extern uint8_t data_4g;//服务器消息标识符
void init_4g(void);
void tx_4g(void);
void rx_4g(void);

#endif
