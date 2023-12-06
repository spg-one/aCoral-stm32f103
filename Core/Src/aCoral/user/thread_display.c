#include "thread.h"
#include "policy.h"
#include "hal.h"
#include "shell.h"
#include <stdio.h>

void display_thread(int argc,char **argv){	
	acoral_list_t *head,*tmp;
	acoral_thread_t * thread;
	head=&acoral_threads_queue;
	acoral_print("\t\tSystem Thread Information\r\n");
	acoral_print("------------------------------------------------------\r\n");
	acoral_print("Name\t\tType\t\tState\t\tPrio\r\n");
	long level = acoral_enter_critical();

	for(tmp=head->next;tmp!=head;tmp=tmp->next){
		thread=list_entry(tmp,acoral_thread_t,global_list);
		acoral_print("%s\t\t",thread->name);
		switch(thread->policy){
			case ACORAL_SCHED_POLICY_COMM:
				acoral_print("Common\t\t");
				break;
			case ACORAL_SCHED_POLICY_PERIOD:
				acoral_print("Period\t\t");
				break;
			default:
				break;
		}
		if(thread->state&ACORAL_THREAD_STATE_RUNNING)
				acoral_print("Running\t\t");
		else if(thread->state&ACORAL_THREAD_STATE_READY)
				acoral_print("Ready\t\t");
		else if(thread->state&ACORAL_THREAD_STATE_DELAY)
				acoral_print("Delay\t\t");
		else if(thread->state&ACORAL_THREAD_STATE_SUSPEND)
				acoral_print("Sleep\t\t");
		else if(thread->state==ACORAL_THREAD_STATE_EXIT)
				acoral_print("Freeze\t\t");
		else
				acoral_print("Error\t\t");
		
		acoral_print("%d\t\t",thread->prio);
		acoral_print("\r\n");
	}
	acoral_print("------------------------------------------------------\r\n");

	acoral_exit_critical(level);
}

acoral_shell_cmd_t dt_cmd={
	"dt",
	(void*)display_thread,
	"View all thread info",
	NULL
};
