#ifndef USER_H
#define USER_H
#include "cmd.h"
#include "shell.h"
#include "stm32f1xx_hal.h"
#include "lora.h"
#include "dht11.h"
#include "light.h"
#include "run_4g.h"
#include "HcSr04.h"
#include "Adxl345.h"

extern uint8_t data_ready;  //某位为1则说明该传感器采集到数据，0则没有，从低位到高位一次代表加速度传感器、温湿度传感器
#endif