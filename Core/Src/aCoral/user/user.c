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
	
	acoral_period_policy_data_t* data;
	data = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data->prio = 5;
	data->prio_type = ACORAL_HARD_PRIO;
	data->time = 1000;
	acoral_create_thread(get_distance_thread, 512, NULL, "tmp_humi_thread", NULL, ACORAL_SCHED_POLICY_PERIOD, data); //超声波测距
	
	acoral_period_policy_data_t* data2;
	data2 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data2->prio = 5;
	data2->prio_type = ACORAL_HARD_PRIO;
	data2->time = 1000;
	acoral_create_thread(get_temp_humi_thread,512,NULL,"tmp_humi_thread",NULL,ACORAL_SCHED_POLICY_PERIOD,data2); //温湿度线程+OLED显示


	// data->prio = 5;
	// data->time = 500;
	// acoral_create_thread(LightSensor_thread,512,NULL,"light_sensor_thread",NULL,ACORAL_SCHED_POLICY_PERIOD,data);//光敏线程
	
	acoral_period_policy_data_t* data1;
	data1 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data1->prio = 4;
	data1->prio_type = ACORAL_HARD_PRIO;
	data1->time = 3000;
	if(EnableMaster == true)	//当前为主机
	{
		memcpy(Buffer,MY_TEST_Msg,6);
		Radio->SetTxPacket( Buffer, sizeof(Buffer) );                   //主机中先进行一次发送，Radio->Process()调度后回进入RF_TX_DONE状态
		acoral_create_thread(master,512,NULL,"master",NULL,ACORAL_SCHED_POLICY_PERIOD,data1);
	}
	else						//当前为从机
	{	
		acoral_print("this is slave\r\n");
		Radio->StartRx();               //从机进入接收状态，当前lora设置为连续接收模式，若要更改为单一接收或其他模式需要更改sx1276LoRa.c中的LoRaSettings相关设置
		acoral_create_thread(slave,512,NULL,"test",NULL,ACORAL_SCHED_POLICY_PERIOD,data1);
	}


	acoral_period_policy_data_t* data3;
	data3 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data3->prio = 5;
	data3->prio_type = ACORAL_HARD_PRIO;
	data3->time = 1000;
	acoral_create_thread(rx_4g,512,NULL,"rx_4g",NULL,ACORAL_SCHED_POLICY_PERIOD,data3);

	acoral_period_policy_data_t* data4;
	data4 = acoral_malloc(sizeof(acoral_period_policy_data_t));
	data4->prio = 3;
	data4->prio_type = ACORAL_HARD_PRIO;
	data4->time = 5000;
	acoral_create_thread(tx_4g,512,NULL,"tx_4g",NULL,ACORAL_SCHED_POLICY_PERIOD,data4);


	
}
