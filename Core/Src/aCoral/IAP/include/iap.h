/**
 ****************************************************************************************************
 * @file        iap.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2020-05-12
 * @brief       IAP 代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20200512
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef __IAP_H
#define __IAP_H

#include "sys.h"

/* 开启升级功能 */
#define IAP
/* 程序1设置的中断向量表偏移地址 */
#define FLASH_APP1_DIFF 0x5000  
/* 程序2设置的中断向量表偏移地址 */
#define FLASH_APP2_DIFF 0x1E000


typedef void (*iapfun)(void);                   /* 定义一个函数类型的参数 */

#define FLASH_APP1_ADDR         0x08005000      /* 第一个应用程序起始地址(存放在内部FLASH)
                                                 * 保留 0X08000000~0X0800FFFF(64KB) 的空间为 Bootloader 使用
                                                 */
#define FLASH_APP2_ADDR         0x0801E000


void iap_load_app(uint32_t appxaddr);   /* 跳转到APP程序执行 */
void iap_write_appbin(uint32_t appxaddr,uint8_t *appbuf,uint32_t applen);   /* 在指定地址开始,写入bin */

#endif







































