/**
 ****************************************************************************************************
 * @file        stmflash.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2020-04-26
 * @brief       STM32内部FLASH读写 驱动代码
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
 * V1.0 20200426
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "stmflash.h"
/**
 * @brief       从指定地址读取一个半字 (16位数据)
 * @param       faddr   : 读取地址 (此地址必须为2的倍数!!)
 * @retval      读取到的数据 (16位)
 */
uint16_t stmflash_read_halfword(uint32_t faddr)
{
    return *(volatile uint16_t *)faddr;
}

/**
 * @brief       从指定地址开始读出指定长度的数据
 * @param       raddr : 起始地址
 * @param       pbuf  : 数据指针
 * @param       length: 要读取的半字(16位)数,即2个字节的整数倍
 * @retval      无
 */
void stmflash_read(uint32_t raddr, uint16_t *pbuf, uint16_t length)
{
    uint16_t i;

    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_halfword(raddr);    /* 读取2个字节 */
        raddr += 2;     /* 偏移2个字节 */
    }
}

/**
 * @brief       不检查的写入
                这个函数的假设已经把原来的扇区擦除过再写入
 * @param       waddr   : 起始地址 (此地址必须为2的倍数!!,否则写入出错!)
 * @param       pbuf    : 数据指针
 * @param       length  : 要写入的 半字(16位)数
 * @retval      无
 */
void stmflash_write_nocheck(uint32_t waddr, uint16_t *pbuf, uint16_t length)
{
    uint16_t i;
    
    for (i = 0; i < length; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, waddr, pbuf[i]);
        waddr += 2;     /* 指向下一个半字 */
    }
}

/**
 * @brief       在FLASH 指定位置, 写入指定长度的数据(自动擦除)
 *   @note      该函数往 STM32 内部 FLASH 指定位置写入指定长度的数据
 *              该函数会先检测要写入的扇区是否是空(全0XFFFF)的?, 如果
 *              不是, 则先擦除, 如果是, 则直接往扇区里面写入数据.
 *              数据长度不足扇区时，自动被回擦除前的数据
 * @param       waddr   : 起始地址 (此地址必须为2的倍数!!,否则写入出错!)
 * @param       pbuf    : 数据指针
 * @param       length  : 要写入的 半字(16位)数
 * @retval      无
 */
uint16_t g_flashbuf[STM32_SECTOR_SIZE / 2]; /* 最多是2K字节 */
void stmflash_write(uint32_t waddr, uint16_t *pbuf, uint16_t length)
{
    uint32_t secpos;    /* 扇区编号 */
    uint16_t secoff;    /* 扇区内偏移地址(16位字计算) */
    uint16_t secremain; /* 扇区内剩余地址(16位字计算) */
    uint16_t i;
    uint32_t offaddr;   /* 去掉0X08000000后的地址 */
    FLASH_EraseInitTypeDef flash_eraseop;
    uint32_t erase_addr;   /* 擦除错误，这个值为发生错误的扇区地址 */

    if (waddr < STM32_FLASH_BASE || (waddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
    {
        return; /* 非法地址 */
    }

    HAL_FLASH_Unlock();                         /* FLASH解锁 */

    offaddr = waddr - STM32_FLASH_BASE;         /* 实际偏移地址. */
    secpos = offaddr / STM32_SECTOR_SIZE;       /* 得到扇区编号 */
    secoff = (offaddr % STM32_SECTOR_SIZE) / 2; /* 在扇区内的偏移(2个字节为基本单位.) */
    secremain = STM32_SECTOR_SIZE / 2 - secoff; /* 扇区剩余空间大小 */
    
    if (length <= secremain)
    {
        secremain = length;     /* 不大于该扇区范围 */
    }

    while (1)
    {
        stmflash_read(secpos * STM32_SECTOR_SIZE + STM32_FLASH_BASE, g_flashbuf, STM32_SECTOR_SIZE / 2); /* 读出整个扇区的内容 */
        
        for (i = 0; i < secremain; i++)         /* 校验数据 */
        {
            if (g_flashbuf[secoff + i] != 0XFFFF)
            {
                break; /* 需要擦除 */
            }
        }
        
        if (i < secremain) /* 需要擦除 */
        { 
            flash_eraseop.TypeErase = FLASH_TYPEERASE_PAGES;    /* 选择页擦除 */
            flash_eraseop.NbPages = 1;                          /* 要擦除的页数 */
            flash_eraseop.PageAddress = secpos * STM32_SECTOR_SIZE + STM32_FLASH_BASE;  /* 要擦除的起始地址 */
            HAL_FLASHEx_Erase( &flash_eraseop, &erase_addr);

            for (i = 0; i < secremain; i++)     /* 复制 */
            {
                g_flashbuf[i + secoff] = pbuf[i];
            }
            stmflash_write_nocheck(secpos * STM32_SECTOR_SIZE + STM32_FLASH_BASE, g_flashbuf, STM32_SECTOR_SIZE / 2); /* 写入整个扇区 */
        }
        else
        {
            stmflash_write_nocheck(waddr, pbuf, secremain); /* 写已经擦除了的,直接写入扇区剩余区间. */
        }
        
        if (length == secremain)
        {
            break; /* 写入结束了 */
        }
        else       /* 写入未结束 */
        {
            secpos++;                   /* 扇区地址增1 */
            secoff = 0;                 /* 偏移位置为0 */
            pbuf += secremain;          /* 指针偏移 */
            waddr += secremain * 2;     /* 写地址偏移(16位数据地址,需要*2) */
            length -= secremain;        /* 字节(16位)数递减 */
            
            if (length > (STM32_SECTOR_SIZE / 2))
            {
                secremain = STM32_SECTOR_SIZE / 2; /* 下一个扇区还是写不完 */
            }
            else
            {
                secremain = length; /* 下一个扇区可以写完了 */
            }
        }
    }

    HAL_FLASH_Lock(); /* 上锁 */
}

/******************************************************************************************/
/* 测试用代码 */

/**
 * @brief       测试写数据(写1个字)
 * @param       waddr : 起始地址
 * @param       wdata : 要写入的数据
 * @retval      读取到的数据
 */
void test_write(uint32_t waddr, uint16_t wdata)
{
    stmflash_write(waddr, &wdata, 1); /* 写入一个半字 */
}
