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
	data->prio = 5;
	data->prio_type = ACORAL_HARD_PRIO;
	data->time = 10;

	lora_init();
	if(EnableMaster == true)	//当前为主机
	{
		
		Radio->SetTxPacket( Buffer, sizeof(Buffer) );                   //主机中先进行一次发送，Radio->Process()调度后回进入RF_TX_DONE状态
		acoral_create_thread(master,512,NULL,"master",NULL,ACORAL_SCHED_POLICY_PERIOD,data);
	}
	else						//当前为从机
	{	
		acoral_print("this is slave\r\n");
		Radio->StartRx();               //从机进入接收状态，当前lora设置为连续接收模式，若要更改为单一接收或其他模式需要更改sx1276LoRa.c中的LoRaSettings相关设置
		acoral_create_thread(slave,512,NULL,"test",NULL,ACORAL_SCHED_POLICY_PERIOD,data);
	}
	
}
