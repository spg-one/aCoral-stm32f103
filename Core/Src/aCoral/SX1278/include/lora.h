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



/*中心站终端标识符*/
// #define MASTER
#define SLAVE

typedef struct 
{
    uint8_t data_type;           //0x01表示数据，0x02表示命令
    uint8_t master_id;
    uint8_t slave_device_id;
    uint8_t data_significant;

    uint8_t command_significant;
    uint8_t update_temp_humi_period;
    uint8_t update_distance_period;
    uint8_t update_acceleration_period;

    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t humi_int;
    uint8_t humi_dec;

    uint8_t temp_humi_period;
    uint8_t distance_period;
    uint8_t acceleration_period;

    float distance;
    float acceleration_x;
    float acceleration_y;
    float acceleration_z;

} data_buffer;





  
extern data_buffer  Buffer;        // 中心站采集数据缓冲区Buffer
extern data_buffer  slave_Data;    // 终端采集数据缓冲区      
extern data_buffer rx_cmd;

extern tRadioDriver *Radio;                 // lora操作指针，所有的函数调用和消息接发都通过此变量操作，tRadioDriver类型定义在头文件radio.h中定义

extern uint8_t master_device_id;
extern uint8_t master_data;                 //中心站是否接收到终端数据标识符





void lora_init();                           //Radio初始化
void master_tx(void *args);                 //中心站发送服务函数
void master_rx(void *args);                 //中心站接收服务函数
void slave_tx(void *args);                  //终端发送服务函数    
void slave_rx(void *args);                  //终端接收服务函数    
void test();



#endif
