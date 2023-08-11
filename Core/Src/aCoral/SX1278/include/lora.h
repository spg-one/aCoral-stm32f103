/**
* @file: lora.h
* @author: 王若宇
* @brief: 导入sx1278芯片所需的驱动头文件，并定义lora外部调用函数
* @version: 1.0
* @date: 2023-05-13
* @LastEditTime: 2023-05-13
*/
#ifndef __LORA_H__
#define __LORA_H__
#include"acoral.h"
#include "fifo.h"
#include "platform.h"
#include "radio.h"
#include "spi.h"
#include "sx1276.h"
#include "sx1276Hal.h"
#include "sx1276LoRa.h"
#include "sx1276LoRaMisc.h"




// #define MASTER
#define SLAVE

#define BUFFER_SIZE     8                  // lora消息接收发送用户数据缓冲区Buffer大小
extern uint16_t BufferSize;   
extern uint8_t  Buffer[BUFFER_SIZE];        // lora消息接收发送用户数据缓冲区Buffer
extern tRadioDriver *Radio;                 // lora操作指针，所有的函数调用和消息接发都通过此变量操作，tRadioDriver类型定义在头文件radio.h中定义
extern uint8_t EnableMaster;                // 主机、从机标识符，true表示主机，false为从机
extern uint8_t rx_cmd[6];
extern uint8_t master_data;
extern uint8_t slave_Data[8]; 



void lora_init();                           //Radio初始化
void master_tx(void *args);                               //中心站发送服务函数
void master_rx(void *args);                               //中心站接收服务函数
void slave_tx(void *args);                                //终端发送服务函数    
void slave_rx(void *args);                                //终端接收服务函数    

#endif
