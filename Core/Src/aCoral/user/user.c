#include "acoral.h"
#include "shell.h"


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
	acoral_period_policy_data_t* data;
	data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data->prio = 20;
	data->prio_type = ACORAL_HARD_PRIO;
	data->time = 1;

	lora_init();
	if(EnableMaster == true)	//当前为主机
	{
		acoral_print("this is master\r\n");
		acoral_create_thread(master,512,NULL,"master",NULL,ACORAL_SCHED_POLICY_PERIOD,data);
	}
	else						//当前为从机
	{	
		acoral_print("this is slave\r\n");
		acoral_create_thread(slave,512,NULL,"test",NULL,ACORAL_SCHED_POLICY_PERIOD,data);
	}
	
}
