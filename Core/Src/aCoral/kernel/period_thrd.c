/**
 * @file period_thrd.c
 * @author 王彬浩 (SPGGOGOGO@outlook.com)
 * @brief kernel层，周期线程
 * @version 1.0
 * @date 2023-05-08
 * @copyright Copyright (c) 2023
 * @revisionHistory
 *  <table>
 *   <tr><th> 版本 <th>作者 <th>日期 <th>修改内容
 *   <tr><td> 0.1 <td>jivin <td> 2010-03-08 <td>Created
 *   <tr><td> 1.0 <td>王彬浩 <td> 2023-05-08 <td>Standardized
 *  </table>
 */

#include "thread.h"
#include "hal.h"
#include "policy.h"
#include "mem.h"
#include "timer.h"
#include "period_thrd.h"
#include "int.h"
#include <stdio.h>

acoral_list_t period_delay_queue; //周期线程专用延时队列，只要是周期线程，就会被挂载到这个队列上，延时时间就是周期，每次周期过后重新挂载
int period_policy_thread_init(acoral_thread_t *thread,void (*route)(void *args),void *args,void *data){
	unsigned int prio;
	acoral_period_policy_data_t *policy_data;
	period_private_data_t *private_data;
	if(thread->policy==ACORAL_SCHED_POLICY_PERIOD){
		policy_data=(acoral_period_policy_data_t *)data;
		prio=policy_data->prio;
		if(policy_data->prio_type==ACORAL_NONHARD_PRIO){
			prio+=ACORAL_NONHARD_RT_PRIO_MAX;
			if(prio>=ACORAL_NONHARD_RT_PRIO_MIN)
				prio=ACORAL_NONHARD_RT_PRIO_MIN-1;
		}
		thread->prio=prio;
		//不用acoral_malloc2
		private_data=(period_private_data_t *)acoral_malloc(sizeof(period_private_data_t));
		if(private_data==NULL){
			acoral_print("No level2 mem space for private_data:%s\n",thread->name);
			long level = acoral_enter_critical();
			acoral_release_res((acoral_res_t *)thread);
			acoral_exit_critical(level);
			return -1;
		}
		private_data->time=policy_data->time;
		private_data->route=route;
		private_data->args=args;
		thread->private_data=private_data;
	}
	if(acoral_thread_init(thread,route,period_thread_exit,args)!=0){
		acoral_print("No period thread stack:%s\r\n",thread->name);
		long level = acoral_enter_critical();
		acoral_release_res((acoral_res_t *)thread);
		acoral_exit_critical(level);
		return -1;
	}
        /*将线程就绪，并重新调度*/
	acoral_resume_thread(thread);
	long level = acoral_enter_critical();
	period_thread_delay(thread,((period_private_data_t *)thread->private_data)->time);
	acoral_exit_critical(level);
	return thread;
}

void period_policy_thread_release(acoral_thread_t *thread){
	acoral_free(thread->private_data);	
}

//差分队列添加
void acoral_periodqueue_add(acoral_thread_t *new){
	acoral_list_t   *tmp,*head;
	acoral_thread_t *thread;
	int  delay2;
	int  delay= new->delay;
	head=&period_delay_queue;
	new->state|=ACORAL_THREAD_STATE_DELAY;
	for (tmp=head->next;delay2=delay,tmp!=head; tmp=tmp->next){
		thread = list_entry (tmp, acoral_thread_t, waiting);
		delay  = delay-thread->delay;
		if (delay < 0)
			break;
	}
	new->delay = delay2;
	acoral_list_add(&new->waiting,tmp->prev);
	/* 插入等待任务后，后继等待任务时间处理*/
	if(tmp != head){
		thread = list_entry(tmp, acoral_thread_t, waiting);
		thread->delay-=delay2;
	}
}

/**
* @author: 贾苹
* @brief: 差分队列删除一个线程
* @version: 1.0
* @date: 2023-09-12
*/
void acoral_periodqueue_remove(acoral_thread_t *thread_to_remove)
{
	acoral_list_t *tmp;
    acoral_thread_t *thread;
    int delay_to_remove = thread_to_remove->delay;

	for (tmp = period_delay_queue.next; tmp != &period_delay_queue; tmp = tmp->next) {
        thread = list_entry(tmp, acoral_thread_t, waiting);

		//找到要删除的TCB
		if (thread == thread_to_remove) {
			// 更新后继任务的延迟时间
            if (tmp->next != &period_delay_queue) {
                acoral_thread_t *next_thread = list_entry(tmp->next, acoral_thread_t, waiting);
                next_thread->delay += delay_to_remove;
            }
			// acoral_list_del(tmp); // 从队列中删除
			acoral_list_del(&thread->waiting);
			//thread->state &= ~ACORAL_THREAD_STATE_DELAY; // 更新任务状态
            break;
		}
	}
}



//重新挂载周期
void period_thread_delay(acoral_thread_t* thread,unsigned int time){
	thread->delay=TIME_TO_TICKS(time);
	acoral_periodqueue_add(thread);
}

void period_delay_deal(){
	acoral_list_t *tmp,*tmp1,*head;
	acoral_thread_t * thread;
	period_private_data_t * private_data;
	head=&period_delay_queue;
	if(acoral_list_empty(head))
	    	return;
	thread=list_entry(head->next,acoral_thread_t,waiting);
	thread->delay--;
	for(tmp=head->next;tmp!=head;){
		thread=list_entry(tmp,acoral_thread_t,waiting);
		if(thread->delay>0)
		    break;
		private_data=thread->private_data;
		/*防止add判断delay时取下thread*/
		tmp1=tmp->next;
		acoral_list_del(&thread->waiting);
		tmp=tmp1;
		if(thread->state&ACORAL_THREAD_STATE_SUSPEND){ //如果此时是就绪状态，则不用重新挂载到就绪链表上。
			thread->stack=(unsigned int *)((char *)thread->stack_buttom+thread->stack_size-4);
			HAL_STACK_INIT(&thread->stack,private_data->route,period_thread_exit,private_data->args);
			acoral_rdy_thread(thread);
			
		}
		period_thread_delay(thread,private_data->time);
	
	}
}

void period_thread_exit(){
	acoral_suspend_self();
}

acoral_sched_policy_t period_policy;
void period_policy_init(void){
	acoral_init_list(&period_delay_queue);
	period_policy.type=ACORAL_SCHED_POLICY_PERIOD;
	period_policy.policy_thread_init=period_policy_thread_init;
	period_policy.policy_thread_release=period_policy_thread_release;
	period_policy.delay_deal=period_delay_deal;
	period_policy.name="period";
	acoral_register_sched_policy(&period_policy);
}
