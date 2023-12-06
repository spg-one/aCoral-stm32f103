#ifndef USER_H
#define USER_H
#include "cmd.h"
#include "shell.h"
#include "stm32f1xx_hal.h"
#include "rtc.h"
#include "lora.h"
#include "dht11.h"
#include "run_4g.h"
#include "HcSr04.h"
#include "Adxl345.h"


/*现有线程id*/
typedef struct
{
    acoral_thread_t *temp_humi_thread;
    acoral_thread_t *distance_thread;
    acoral_thread_t *acceleration_thread;
    #if defined(MASTER)
		acoral_thread_t *master_rx_thread;
	#endif

	#if defined(SLAVE)
		acoral_thread_t *slave_tx_thread;
	#endif
} threads;

/*现有线程周期*/
typedef struct
{
    uint8_t period_significant;
    uint8_t temp_humi;
    uint8_t distance;
    uint8_t acceleration;
} threads_period;



extern uint8_t data_ready;  //某位为1则说明该传感器采集到数据，0则没有，从低位到高位一次代表加速度传感器、温湿度传感器

extern threads_period cur_period;
extern threads cur_threads; 

extern uint32_t sync_tick;

extern uint32_t interval_period; //间隔周期，同步时需要
extern uint32_t sync_start; //同步开始时间点
extern uint8_t  sync_flag; //升级标志，为1时，同步syn_tick；为0时，不进行任何任务同步；为2时，syn_tick同步成功
extern uint32_t tick_diff_threshold; //差的阈值，中心站与终端的sync_tick要小于这一阈值，才表明sync_tick数据同步成功

extern uint8_t  slave_num;
extern uint32_t interval_period;   // 间隔周期 = 中心站接收周期
extern uint32_t sync_start;			   // 同步开始时间 > 最大采集周期(所有终端)

extern int master_sync_thread_id;
extern int master_reload_thread_id;
extern int slave_sync_thread_id;
extern int slave_reload_thread_id;



#endif