/**
 * @file hal_thread.h
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，线程相关头文件
 * @version 1.0
 * @date 2022-07-17
 * @copyright Copyright (c) 2022
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-07-17 <td>Standardized 
 *  </table>
 */

#ifndef HAL_THREAD_H
#define HAL_THREAD_H

/**
 * @brief aCoral线程上下文context在硬件层面的描述
 */
typedef struct {
	unsigned int primask;
	unsigned int r4; ///<通用寄存器
	unsigned int r5; ///<通用寄存器
	unsigned int r6; ///<通用寄存器
	unsigned int r7; ///<通用寄存器
	unsigned int r8; ///<通用寄存器
	unsigned int r9; ///<通用寄存器
	unsigned int r10; ///<通用寄存器
	unsigned int r11; ///<通用寄存器
	unsigned int r0; ///<通用寄存器
	unsigned int r1; ///<通用寄存器
	unsigned int r2; ///<通用寄存器
	unsigned int r3; ///<通用寄存器
	unsigned int r12; ///<通用寄存器
	unsigned int lr; ///<链接寄存器
	unsigned int pc; ///<程序计数器
	unsigned int cpsr; ///<程序状态寄存器
	
}hal_ctx_t;

void hal_stack_init(unsigned int **stk,void (*route)(void),void (*exit)(void),void *args);

//线程相关的硬件抽象接口 //TODO全大写为了和汇编接口统一
#define HAL_STACK_INIT(stack,route,exit,args) hal_stack_init(stack,route,exit,args)

#endif