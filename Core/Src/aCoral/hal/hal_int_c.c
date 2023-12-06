/**
 * @file hal_int_c.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，中断二级入口、中断开关以及中断初始化c代码
 * @version 1.0
 * @date 2022-06-23
 * @copyright Copyright (c) 2023
 *
 * <table>
 * <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td> 0.1 <td>jivin <td>2010-3-8 <td>Created
 * <tr><td> 1.0 <td>王彬浩 <td>2022-06-23 <td>Standardized
 * </table>
 */

#include "hal_int.h"
#include "lsched.h"

///中断嵌套数
unsigned int intr_nesting;

// void (*intr_c_entry)(signed int);
// void hal_intr_set_entry(void (*isr)(signed int)){
// 	intr_c_entry=isr;
// }

void hal_intr_unmask(int vector)
{
	// TODO riscv-加上
}

void hal_intr_mask(int vector)
{
	// TODO riscv-加上
}

void hal_intr_nesting_init_comm()
{
	intr_nesting = 0;
}

unsigned int hal_get_intr_nesting_comm()
{
	return intr_nesting;
}

void hal_intr_nesting_dec_comm()
{
	if (intr_nesting > 0)
		intr_nesting--;
}

void hal_intr_nesting_inc_comm()
{
	intr_nesting++;
}

void hal_sched_bridge_comm()
{
	long level = HAL_ENTER_CRITICAL(); //SPG这里关了中断，线程切换后，新的线程默认是开启中断的状态了
	
	acoral_real_sched();
	HAL_EXIT_CRITICAL(level);
}

void hal_intr_exit_bridge_comm()
{

	long level = HAL_ENTER_CRITICAL();
	acoral_real_intr_sched();
	HAL_EXIT_CRITICAL(level);
}
