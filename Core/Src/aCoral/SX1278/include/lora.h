/**
* @file: lora.h
* @author: 贾苹
* @brief: 导入sx1278芯片所需的驱动头文件，并定义lora外部调用函数
* @version: 2.0
* @date: 2023-09-06
* @LastEditTime: 2023-09-06
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
    //17字节
    uint8_t data_type;                  //0x01表示数据，0x02表示数据正确，0x03表示数据错误，0x04表示升级命令，0x05表示同步命令，0x06表示更改采集周期命令
    uint8_t master_id;                  //中心站id 0x00(0000 0000)共八位，前两位表示设备类型，00表示中心站，01表示终端，后六位代表地址
    uint8_t slave_device_id;            //终端id 0x40(0100 0000) 0x41(0100 0001)
    uint8_t data_significant;           //数据有效位，每一位代表当前数据是否有效，从低到高位依次是：超声波传感器、温湿度传感器、加速度传感器

    uint8_t change_period_significant;  //更改命令有效位，从低到高依次表示超声波、温湿度、加速度周期是否更改，1表示是，0表示否。
    uint8_t update_temp_humi_period;    //要更改的传感器采集周期
    uint8_t update_distance_period;     //都是以s为单位
    uint8_t update_acceleration_period;

    uint8_t temp_int;                   //温度整数
    uint8_t temp_dec;                   //温度小数
    uint8_t humi_int;
    uint8_t humi_dec;

    uint8_t temp_humi_period;           //当前传感器采集周期
    uint8_t distance_period;
    uint8_t acceleration_period;

    uint8_t package_sum;                //升级包个数
    uint32_t package_size;              //升级包大小

    //4字节
    uint32_t sync_tick;                 //同步tick

    //12字节
    float distance;
    float acceleration_x;
    float acceleration_y;
    float acceleration_z;

    uint32_t temp_collect_time;
    uint32_t distance_collect_time;
    uint32_t acceleration_collect_time;

} data_buffer;





  
extern data_buffer  Buffer;        // 中心站采集数据缓冲区Buffer
extern data_buffer  slave_Data;    // 终端采集数据缓冲区      
extern data_buffer rx_cmd;

extern tRadioDriver *Radio;                 // lora操作指针，所有的函数调用和消息接发都通过此变量操作，tRadioDriver类型定义在头文件radio.h中定义

extern uint8_t master_device_id;
extern uint8_t master_data;                 //中心站是否接收到终端数据标识符





void lora_init();                           //Radio初始化
uint8_t master_tx();                        //中心站发送服务函数
void master_rx(void *args);                 //中心站接收服务函数
void slave_tx(void *args);                  //终端发送服务函数    
uint8_t slave_rx_resend();                  //终端接收并重传
uint8_t slave_rx();                         //终端接收不重传
void period_change();                       //线程周期更改函数

#if defined(SLAVE)
    extern uint8_t slave_device_id;         //用于时钟同步时计算是几号终端设备
#endif

#endif
