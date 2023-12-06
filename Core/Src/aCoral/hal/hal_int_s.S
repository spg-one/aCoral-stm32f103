/**
 * @file hal_int_s.S
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，中断一级入口、底层开关中断操作
 * @version 1.0
 * @date 2022-06-24
 * @copyright Copyright (c) 2022
 * @revisionHistory 
 *  <table> 
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容 
 *   <tr><td> 0.1 <td>jivin <td>2010-03-08 <td>Created 
 *   <tr><td> 1.0 <td>王彬浩 <td> 2022-06-24 <td>Standardized 
 *  </table>
 */




   .global     HAL_INTR_DISABLE
   .global     HAL_INTR_ENABLE
   .global     HAL_INTR_DISABLE_SAVE
   .global     HAL_INTR_RESTORE
   .extern     IRQ_stack

HAL_INTR_ENABLE:
    MSR PRIMASK,R0
    mov pc ,lr

HAL_INTR_DISABLE:
    MRS R0,PRIMASK
    CPSID i
    mov pc ,lr






