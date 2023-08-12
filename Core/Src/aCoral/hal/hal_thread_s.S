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

.text
.equ	NVIC_INT_CTRL,   0xE000ED04   /* interrupt control state register */
.equ	NVIC_SYSPRI2,    0xE000ED20   /* system priority register (2) */
.equ	NVIC_PENDSV_PRI, 0x00FF0000   /* PendSV priority value (lowest) */
.equ	NVIC_PENDSVSET,  0x10000000   /* value to trigger PendSV exception*/

.global HAL_INTR_CTX_SWITCH
.global HAL_CONTEXT_SWITCH
.global HAL_INTR_SWITCH_TO
.global HAL_SWITCH_TO
.global HAL_SCHED_BRIDGE
.global PENDSV_CALL
.global HAL_START_OS

.syntax unified

.type HAL_SWITCH_TO, %function

HAL_START_OS:
	LDR     R1,=Msp_stack
	MSR     MSP,R1			    @重置主栈指针，主栈将用于异常和中断
	MOV     R1,#2
	MSR     CONTROL,R1          @切换到用户线程模式，栈指针用PSP
	ldr     r0,[r0]
	msr     psp,r0
	ldmfd   sp!, {r0,r4-r11}
	msr  primask,r0
	ldmfd   sp!, {r0-r3}
	ldmfd   sp!,{r4-r7}
	mov  lr,r5  
	mov  pc,r6  


HAL_SCHED_BRIDGE:
	ldr     r0,  =NVIC_INT_CTRL          @触发pendsv中断切换进程
	ldr     r1,  =NVIC_PENDSVSET
	CPSIE   I                            @必须开中断
	str     r1,  [r0]
	bx lr



/*HAL_SWITCH_TO:
    ldr r0, [r0]
	mov sp, r0	
	pop {r0}
	msr xpsr, r0
	pop {r0-r12,lr}				// Pop the registers. 
	pop {pc}					// Reglist can't contain pc and lr simultaneously
	

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
	pop {r0-r12,lr}				// Pop the registers. 
	pop {pc}					// Reglist can't contain pc and lr simultaneously 

*/
HAL_INTR_SWITCH_TO:
HAL_SWITCH_TO:
	ldr     r1,[r0]     @取待切入线程的psp
	msr     psp,r1   
	bx      lr

HAL_INTR_CTX_SWITCH:
HAL_CONTEXT_SWITCH:
	mrs 	r2,psp
	str 	r2,[r0]	
	ldr     r2,[r1]     @取待切入线程的psp
	msr     psp,r2      @待切入线程的当前栈指针写入psp，中断返回恢复上下文将以此为准
	bx      lr
	
PENDSV_CALL:
    mrs r8,primask
    CPSID   I
	push    {r0,lr}		
    mrs     r0,psp
    stmfd   r0!,{r1,r4-r11}       
    msr     psp,r0
    bl acoral_real_intr_sched
	pop     {r0,lr}
    mrs     r0,psp
    ldmfd   r0!,{r1,r4-r11}
    msr     psp,r0
    msr  primask,r8
    bx      lr


