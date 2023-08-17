#include "acoral.h"
#include "shell.h"

uint8_t data_ready = 0;
threads_period cur_period;
threads cur_threads = {0};





void user_main(void)
{
	
	acoral_period_policy_data_t* get_distance_thread_data;
	get_distance_thread_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	get_distance_thread_data->prio = 5;
	get_distance_thread_data->prio_type = ACORAL_HARD_PRIO;
	get_distance_thread_data->time = 20000;
	cur_period.distance = (get_distance_thread_data->time)/1000;
	cur_threads.distance_thread = acoral_create_thread(get_distance_thread, 512, NULL, "get_distance_thread", NULL, ACORAL_SCHED_POLICY_PERIOD, get_distance_thread_data); //超声波测距
	
	acoral_period_policy_data_t* tmp_humi_thread_data;
	tmp_humi_thread_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	tmp_humi_thread_data->prio = 5;
	tmp_humi_thread_data->prio_type = ACORAL_HARD_PRIO;
	tmp_humi_thread_data->time = 20000;
	cur_period.temp_humi = (tmp_humi_thread_data->time)/1000;
	cur_threads.temp_humi_thread = acoral_create_thread(get_temp_humi_thread,512,NULL,"tmp_humi_thread",NULL,ACORAL_SCHED_POLICY_PERIOD,tmp_humi_thread_data); //温湿度线程+OLED显示

	acoral_period_policy_data_t* getXYZAxisAccelerationsThread_data;
	getXYZAxisAccelerationsThread_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	getXYZAxisAccelerationsThread_data->prio = 5;
	getXYZAxisAccelerationsThread_data->prio_type = ACORAL_HARD_PRIO;
	getXYZAxisAccelerationsThread_data->time = 20000;
	cur_period.acceleration = (getXYZAxisAccelerationsThread_data->time)/1000;
	cur_threads.acceleration_thread = acoral_create_thread(getXYZAxisAccelerationsThread, 1536, NULL, "getXYZAxisAccelerationsThread", NULL, ACORAL_SCHED_POLICY_PERIOD, getXYZAxisAccelerationsThread_data); //超声波测距
	


	#if defined( MASTER )//当前为中心站
	/*中心站lora发送线程*/
	acoral_period_policy_data_t* master_tx_data;
	master_tx_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	master_tx_data->prio = 3;
	master_tx_data->prio_type = ACORAL_HARD_PRIO;
	master_tx_data->time = 3000;
	acoral_create_thread(master_tx,512,NULL,"master_tx",NULL,ACORAL_SCHED_POLICY_PERIOD,master_tx_data);

	/*中心站lora接收线程*/
	acoral_period_policy_data_t* master_rx_data;
	master_rx_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	master_rx_data->prio = 2;
	master_rx_data->prio_type = ACORAL_HARD_PRIO;
	master_rx_data->time = 5000;                   
	acoral_create_thread(master_rx,512,NULL,"master_rx",NULL,ACORAL_SCHED_POLICY_PERIOD,master_rx_data);
	
	/*中心站4g接收线程*/
	acoral_period_policy_data_t* rx_4g_data;
	rx_4g_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	rx_4g_data->prio = 5;
	rx_4g_data->prio_type = ACORAL_HARD_PRIO;
	rx_4g_data->time = 5000;
	acoral_create_thread(rx_4g,512,NULL,"rx_4g",NULL,ACORAL_SCHED_POLICY_PERIOD,rx_4g_data);

	/*中心站4g发送线程*/
	acoral_period_policy_data_t* tx_4g_data;
	tx_4g_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	tx_4g_data->prio = 3;
	tx_4g_data->prio_type = ACORAL_HARD_PRIO;
	tx_4g_data->time = 2000;
	acoral_create_thread(tx_4g,512,NULL,"tx_4g",NULL,ACORAL_SCHED_POLICY_PERIOD,tx_4g_data);
	#endif

	#if defined( SLAVE )//当前为终端
	/*终端lora发送线程*/
	acoral_period_policy_data_t* slave_tx_data;
	slave_tx_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	slave_tx_data->prio = 3;
	slave_tx_data->prio_type = ACORAL_HARD_PRIO;
	slave_tx_data->time = 2000;
	acoral_create_thread(slave_tx,512,NULL,"slave_tx",NULL,ACORAL_SCHED_POLICY_PERIOD,slave_tx_data);

	/*终端lora接收线程*/
	acoral_period_policy_data_t* slave_rx_data;
	slave_rx_data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	slave_rx_data->prio = 6;
	slave_rx_data->prio_type = ACORAL_HARD_PRIO;
	slave_rx_data->time = 10000;
	acoral_create_thread(slave_rx,512,NULL,"slave_rx",NULL,ACORAL_SCHED_POLICY_PERIOD,slave_rx_data);
	
	#endif

	


	
}
