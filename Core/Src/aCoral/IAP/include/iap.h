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

#define APP1_DIFF         0x5000                  /* 程序1设置的中断向量表偏移地址 */
#define APP2_DIFF         0x1E000                 /* 程序2设置的中断向量表偏移地址 */


#define APP1_ADDR         0x08005000              /* 第一个应用程序起始地址(存放在内部FLASH) */
#define APP2_ADDR         0x0801E000              /* 第二个应用程序起始地址(存放在内部FLASH) */
#define DIFF_ADDR         0x08037000              /* 差分文件存放起始地址 */
#define USER_ADDR         0x0803F800              /* 用户数据存放起始地址(存放在内部FLASH) */

/* 用户数据 */
typedef struct user_data{
    uint16_t upgrade_flag;                      /* 升级标志，如果为0，表示没有升级，跳转到upgrade_number对应的程序即可；如果为1，表示需要升级，做升级操作后，再重启重新进入BootLoader */
    uint16_t current_number;                    /* 当前序号，如果为0就跳转到APP1，如果为1就跳转到APP2  表示现在可以运行的最新的程序 */
    
    uint16_t package_sum;                       /* 升级包总数 */
    uint32_t package_size;                      /* 差分文件总大小，单位B */
    uint16_t package_num;                       /* 当前已下载的升级包序号 */
}user_data_t;


typedef void (*iapfun)(void);                   /* 定义一个函数类型的参数 */


void iap_load_app(uint32_t appxaddr);   /* 跳转到APP程序执行 */
void iap_write_appbin(uint32_t appxaddr,uint8_t *appbuf,uint32_t applen);   /* 在指定地址开始,写入bin */

#endif







































