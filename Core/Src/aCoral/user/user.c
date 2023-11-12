#include "acoral.h"
#include "shell.h"

uint8_t data_ready = 0; 
threads_period cur_period;
threads cur_threads = {0};
uint32_t sync_tick = 0;
uint8_t sync_flag = 0;
uint32_t tick_diff_threshold = 500; //待定？
uint8_t window_diff = 0; //窗口误差


uint8_t slave_num;							// 终端数目
uint32_t interval_period;       			// 间隔周期 = (终端发送周期/终端数目)
uint32_t sync_start = 40000;			   // 同步开始时间 > 最大采集周期(所有终端)


int master_sync_thread_id, master_reload_thread_id, slave_sync_thread_id,slave_reload_thread_id;

#if defined(MASTER)
void master_sync_thread(void *args)
{
    //仅同步syn_tick阶段执行该任务
    while (1)
    {
        if(sync_flag != 1)
        {
            acoral_suspend_self();
        }
        master_rx(args);
        
    }
}

void master_reload_thread(void *args)
{
	while (1)
	{
		if (sync_flag!=1)
		{
			acoral_suspend_self();
		}
		acoral_print("------------reload thread----------\r\n");
		acoral_thread_t *thread;
		period_private_data_t *private_data;

		// 从周期延时队列中删去
		thread = cur_threads.master_rx_thread;
		private_data = thread->private_data;
		acoral_enter_critical();
		acoral_periodqueue_remove(thread);
		// 添加到就绪队列
		if (thread->state & ACORAL_THREAD_STATE_SUSPEND)
		{ // 如果此时是就绪状态，则不用重新挂载到就绪链表上。
			thread->stack = (unsigned int *)((char *)thread->stack_buttom + thread->stack_size - 4);
			HAL_STACK_INIT(&thread->stack, private_data->route, period_thread_exit, private_data->args);
			acoral_rdy_thread(thread);
		}
		// 将新线程挂到周期延时队列
		period_thread_delay(thread, private_data->time); // 更新period_delay_queue队列上的线程的time
		sync_flag = 0;									 // 同步任务结束
		sync_tick = 0;									 // sync_tick清零

		/* 暂时由0变成3，不清零 */
		// sync_flag = 3;
		acoral_exit_critical();
	}
	
}
#endif


#if defined(SLAVE)


/**
* @author: 贾苹
* @brief: 终端同步Syn_Tick的任务
* @version: 2.0
* @date: 2023-09-09
*/
void slave_sync_thread(void *args)
{
	while(1)
	{
		if (sync_flag != 1)
		{
			acoral_suspend_self();
			acoral_print("-------------slave sync tick finish------------\r\n");
		}
		send_data();
		
	}
}


void slave_reload_thread(void *args)
{
	while (1)
	{
		if (sync_flag != 2 || sync_flag != 1)
		{
			acoral_suspend_self();
		}
		acoral_print("------------reload thread----------\r\n");
		acoral_thread_t *thread;
		period_private_data_t *private_data;
		// 从周期延时队列中删去
		thread = cur_threads.slave_tx_thread;
		private_data = thread->private_data;
		acoral_enter_critical();
		acoral_periodqueue_remove(thread);
		// 添加到就绪队列
		if (thread->state & ACORAL_THREAD_STATE_SUSPEND)
		{ // 如果此时是就绪状态，则不用重新挂载到就绪链表上。
			thread->stack = (unsigned int *)((char *)thread->stack_buttom + thread->stack_size - 4);
			HAL_STACK_INIT(&thread->stack, private_data->route, period_thread_exit, private_data->args);
			acoral_rdy_thread(thread);
		}
		// 将新线程挂到周期延时队列
		period_thread_delay(thread, private_data->time); // 更新period_delay_queue队列上的线程的time
		sync_flag = 0;									 // 同步任务结束
		sync_tick = 0;									 // sync_tick清零
		
		/* 暂时由0变成3，不清零 */
		// sync_flag = 3;
		acoral_exit_critical();
	}
	
}

#endif


void user_main(void)
{
	slave_num = 2;	

	uint32_t send_period = 4000;		// 终端发送周期 = 最小采集周期(所有终端)
	uint32_t recv_period = (4000 / slave_num) - window_diff; // 中心站接受周期 =间隔周期 -误差值(待定)
	
	interval_period = (4000 / slave_num);

	acoral_period_policy_data_t* get_distance_thread_data;
	get_distance_thread_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	get_distance_thread_data->prio = 10;
	get_distance_thread_data->prio_type = ACORAL_HARD_PRIO;
	get_distance_thread_data->time = 1000; //ms为单位
	cur_period.distance = (get_distance_thread_data->time)/1000; //以s为单位，记录所有传感器的当前周期
	cur_threads.distance_thread = acoral_create_thread(get_distance_thread, 512, NULL, "get_distance_thread", NULL, ACORAL_SCHED_POLICY_PERIOD, get_distance_thread_data); //超声波测距 记录当前线程的ID

	acoral_period_policy_data_t* tmp_humi_thread_data;
	tmp_humi_thread_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	tmp_humi_thread_data->prio = 10;
	tmp_humi_thread_data->prio_type = ACORAL_HARD_PRIO;
	tmp_humi_thread_data->time = 1000;
	cur_period.temp_humi = (tmp_humi_thread_data->time)/1000;
	cur_threads.temp_humi_thread = acoral_create_thread(get_temp_humi_thread,512,NULL,"tmp_humi_thread",NULL,ACORAL_SCHED_POLICY_PERIOD,tmp_humi_thread_data); //温湿度线程+OLED显示


	acoral_period_policy_data_t* getXYZAxisAccelerationsThread_data;
	getXYZAxisAccelerationsThread_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	getXYZAxisAccelerationsThread_data->prio = 10;
	getXYZAxisAccelerationsThread_data->prio_type = ACORAL_HARD_PRIO;
	getXYZAxisAccelerationsThread_data->time = 1000;
	cur_period.acceleration = (getXYZAxisAccelerationsThread_data->time)/1000;
	cur_threads.acceleration_thread = acoral_create_thread(getXYZAxisAccelerationsThread, 1536, NULL, "getXYZAxisAccelerationsThread", NULL, ACORAL_SCHED_POLICY_PERIOD, getXYZAxisAccelerationsThread_data); //超声波测距


	#if defined( MASTER )//当前为中心站
	/*中心站lora接收线程*/
	acoral_period_policy_data_t* master_rx_data;
	master_rx_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	master_rx_data->prio = 11;
	master_rx_data->prio_type = ACORAL_HARD_PRIO;
	master_rx_data->time = recv_period;                   
	cur_threads.master_rx_thread = acoral_create_thread(master_rx,512,NULL,"master_rx",NULL,ACORAL_SCHED_POLICY_PERIOD,master_rx_data);
	
	/*中心站4g接收线程*/
	acoral_period_policy_data_t* rx_4g_data;
	rx_4g_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	rx_4g_data->prio = 10;
	rx_4g_data->prio_type = ACORAL_HARD_PRIO;
	rx_4g_data->time = 2000;
	acoral_create_thread(rx_4g,512,NULL,"rx_4g",NULL,ACORAL_SCHED_POLICY_PERIOD,rx_4g_data);

	/*中心站4g发送线程*/
	acoral_period_policy_data_t* tx_4g_data;
	tx_4g_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	tx_4g_data->prio = 10;
	tx_4g_data->prio_type = ACORAL_HARD_PRIO;
	tx_4g_data->time = send_period/4;
	acoral_create_thread(tx_4g,512,NULL,"tx_4g",NULL,ACORAL_SCHED_POLICY_PERIOD,tx_4g_data);

	/*中心站同步线程*/
	acoral_comm_policy_data_t* master_sync_data;
	master_sync_data = acoral_malloc(sizeof(acoral_comm_policy_data_t));
	master_sync_data->prio = 11;
	master_sync_data->prio_type = ACORAL_HARD_PRIO;
	master_sync_thread_id = acoral_create_thread(master_sync_thread, 2048, NULL, "master_sync_thread", NULL, ACORAL_SCHED_POLICY_COMM, master_sync_data);
	// acoral_print("---------master_sync_thread_id:%d\r\n",master_sync_thread_id);


	/*中心站重装载线程*/
	acoral_comm_policy_data_t* master_reload_data;
	master_reload_data = acoral_malloc(sizeof(acoral_comm_policy_data_t));
	master_reload_data->prio = 9; // 最大优先级，退出中断后，首先执行
	master_reload_data->prio_type = ACORAL_HARD_PRIO;
	master_reload_thread_id = acoral_create_thread(master_reload_thread, 1024, NULL, "master_reload_thread", NULL, ACORAL_SCHED_POLICY_COMM, master_reload_data);
	// acoral_print("---------master_reload_thread_id:%d\r\n",master_reload_thread_id);

#endif

	#if defined( SLAVE )//当前为终端
	/*终端lora发送线程*/
	acoral_period_policy_data_t* slave_tx_data;
	slave_tx_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	slave_tx_data->prio = 11;
	slave_tx_data->prio_type = ACORAL_HARD_PRIO;
	slave_tx_data->time = send_period; //10min
	cur_threads.slave_tx_thread = acoral_create_thread(slave_tx,1024,NULL,"slave_tx",NULL,ACORAL_SCHED_POLICY_PERIOD,slave_tx_data);

	/*中心站同步线程*/
	acoral_comm_policy_data_t* slave_sync_data;
	slave_sync_data = acoral_malloc(sizeof(acoral_comm_policy_data_t));
	slave_sync_data->prio = 11;
	slave_sync_data->prio_type = ACORAL_HARD_PRIO;
	slave_sync_thread_id = acoral_create_thread(slave_sync_thread, 1024, NULL, "slave_sync_thread", NULL, ACORAL_SCHED_POLICY_COMM, slave_sync_data);
	// acoral_print("---------slave_sync_thread_id:%d\r\n",slave_sync_thread_id);

	/*中心站重装载线程*/
	acoral_comm_policy_data_t* slave_reload_data;
	slave_reload_data = acoral_malloc(sizeof(acoral_comm_policy_data_t));
	slave_reload_data->prio = 9; // 最大优先级，退出中断后，首先执行
	slave_reload_data->prio_type = ACORAL_HARD_PRIO;
	slave_reload_thread_id = acoral_create_thread(slave_reload_thread, 1024, NULL, "slave_reload_thread", NULL, ACORAL_SCHED_POLICY_COMM, slave_reload_data);
	// acoral_print("---------slave_reload_thread_id:%d\r\n",slave_reload_thread_id);
	
	#endif

}

