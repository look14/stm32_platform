#ifndef __TIME_SERVER_H
#define __TIME_SERVER_H

#include "typedefs.h"

typedef struct __system_timestamp {
	u32 sec_count;
	u32 us_count;
}system_timestamp;

typedef void (*p_time_server_proc_run)(void);

typedef struct __delay_run_task {
	u32 ms_count;
	p_time_server_proc_run func;	
} delay_run_task;

#define DEF_TIMESTAMP_US_STEP 		(u32)5  //5us
#define DEF_TIMESTAMP_US_SIZE		(u32)1000000u

#define DELAY_RUN_TASK_SIZE		16
#define DELAY_RUN_TASK_LED1		0
#define DELAY_RUN_TASK_LED2		1
#define DELAY_RUN_TASK_LED3		2
#define DELAY_RUN_TASK_TEST		3
#define DELAY_RUN_TASK_TEST2	4

extern volatile delay_run_task g_delayRunTasks[DELAY_RUN_TASK_SIZE];

extern volatile u32 g_nUserTimeout;
extern volatile system_timestamp g_sysTimestamp;
extern volatile u8 g_isLockSysTimestamp;

extern p_time_server_proc_run g_time_server_proc_run5;
extern p_time_server_proc_run g_time_server_proc_run6;

#define get_system_relative_ms_count(x)		( get_system_relative_us_count(x)/1000u )
#define get_system_relative_sec_count(x)	( get_system_relative_us_count(x)/1000000u )

void time_server_init(void);
void time_server_setDelayRunTask(u8 index, p_time_server_proc_run func, u32 ms);
void time_server_interrupt_proc5(void);

void SetUserTimeout(u32 nTimeout);
u8 IsUserTimeout(void);

void get_system_timestamp(system_timestamp *p_timestamp);
u32 get_system_relative_us_count(system_timestamp *p_timestamp);

void system_delay_ms(u32 n);
void system_delay_10us(u32 n);
void system_delay_100us(u32 n);
void system_delay_us(u32 n);
void system_delay_500ns(void);


#endif
