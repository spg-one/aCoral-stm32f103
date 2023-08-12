#include "acoral.h"
#include "shell.h"

uint8_t data_ready = 0;


void dummy(void *args){
	int i=0;
	while(1){
		i++;
//		LedControl(i);
		acoral_delay_self(1000);
	}
}



void user_main(void)
{
	
	// acoral_period_policy_data_t* data;
	// data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	// data->prio = 5;
	// data->prio_type = ACORAL_HARD_PRIO;
	// data->time = 5000;
	// acoral_create_thread(get_distance_thread, 512, NULL, "get_distance_thread", NULL, ACORAL_SCHED_POLICY_PERIOD, data); //超声波测距
	
	acoral_period_policy_data_t* data2;
	data2 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data2->prio = 5;
	data2->prio_type = ACORAL_HARD_PRIO;
	data2->time = 5000;
	acoral_create_thread(get_temp_humi_thread,512,NULL,"tmp_humi_thread",NULL,ACORAL_SCHED_POLICY_PERIOD,data2); //温湿度线程+OLED显示

	acoral_period_policy_data_t* data5;
	data5 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data5->prio = 5;
	data5->prio_type = ACORAL_HARD_PRIO;
	data5->time = 5000;
	acoral_create_thread(getXYZAxisAccelerationsThread, 1024, NULL, "getXYZAxisAccelerationsThread", NULL, ACORAL_SCHED_POLICY_PERIOD, data5); //超声波测距
	


	#if defined( MASTER )//当前为中心站
	/*中心站lora发送线程*/
	acoral_period_policy_data_t* data0;
	data0 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data0->prio = 3;
	data0->prio_type = ACORAL_HARD_PRIO;
	data0->time = 3000;
	acoral_create_thread(master_tx,512,NULL,"master_tx",NULL,ACORAL_SCHED_POLICY_PERIOD,data0);

	/*中心站lora接收线程*/
	acoral_period_policy_data_t* data1;
	data1 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data1->prio = 4;
	data1->prio_type = ACORAL_HARD_PRIO;
	data1->time = 4000;                   
	acoral_create_thread(master_rx,512,NULL,"master_rx",NULL,ACORAL_SCHED_POLICY_PERIOD,data1);
	
	/*中心站4g接收线程*/
	acoral_period_policy_data_t* data3;
	data3 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data3->prio = 5;
	data3->prio_type = ACORAL_HARD_PRIO;
	data3->time = 5000;
	acoral_create_thread(rx_4g,512,NULL,"rx_4g",NULL,ACORAL_SCHED_POLICY_PERIOD,data3);

	/*中心站4g发送线程*/
	acoral_period_policy_data_t* data4;
	data4 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data4->prio = 3;
	data4->prio_type = ACORAL_HARD_PRIO;
	data4->time = 2000;
	acoral_create_thread(tx_4g,512,NULL,"tx_4g",NULL,ACORAL_SCHED_POLICY_PERIOD,data4);
	#endif

	#if defined( SLAVE )//当前为终端
	/*终端lora发送线程*/
	acoral_period_policy_data_t* data0;
	data0 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data0->prio = 3;
	data0->prio_type = ACORAL_HARD_PRIO;
	data0->time = 3000;
	acoral_create_thread(slave_tx,512,NULL,"slave_tx",NULL,ACORAL_SCHED_POLICY_PERIOD,data0);

	/*终端lora接收线程*/
	acoral_period_policy_data_t* data1;
	data1 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data1->prio = 3;
	data1->prio_type = ACORAL_HARD_PRIO;
	data1->time = 5000;
	acoral_create_thread(slave_rx,1024,NULL,"slave_rx",NULL,ACORAL_SCHED_POLICY_PERIOD,data1);
	
	#endif

	


	
}
