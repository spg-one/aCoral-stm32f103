 /**
 * @file hal_thread_s.s
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief hal层，线程上下文切换操作
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

.global HAL_INTR_CTX_SWITCH
.global HAL_CONTEXT_SWITCH
.global HAL_INTR_SWITCH_TO
.global HAL_SWITCH_TO

.syntax unified

.type HAL_SWITCH_TO, %function
HAL_SWITCH_TO:
    ldr r0, [r0]
	mov sp, r0	
	pop {r0}
	msr xpsr, r0
	pop {r0-r12,lr}				/* Pop the registers. */
	pop {pc}					/* Reglist can't contain pc and lr simultaneously */
	

HAL_INTR_SWITCH_TO:
   
    
HAL_INTR_CTX_SWITCH:
  


HAL_CONTEXT_SWITCH: #//SPG看一下damen的栈里压了什么东西
	push {lr}
	push {r0-r12,lr} 	
	mrs  r4, xpsr
	push {r4}		
	str sp,[r0] 

    ldr r0, [r1]
	mov sp, r0	
	pop {r0}
	msr xpsr, r0
	pop {r0-r12,lr}				/* Pop the registers. */
	pop {pc}					/* Reglist can't contain pc and lr simultaneously */
