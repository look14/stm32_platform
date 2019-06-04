#include <string.h>
#include "time_server.h"
#include "stm32f10x_tim.h"
#include "system.h"

volatile delay_run_task g_delayRunTasks[DELAY_RUN_TASK_SIZE];
volatile u32 g_nUserTimeout;
volatile u8 g_isLockSysTimestamp = 0;
volatile system_timestamp g_sysTimestamp = {0,0};

system_timestamp g_userTimestamp;

p_time_server_proc_run g_time_server_proc_run5;
p_time_server_proc_run g_time_server_proc_run6;

void time_server_init(void)
{
	g_nUserTimeout          = 0;
	g_time_server_proc_run5 = 0;
	g_time_server_proc_run6 = 0;

	memset((void*)g_delayRunTasks, 0, sizeof(g_delayRunTasks));
}

void time_server_setDelayRunTask(u8 index, p_time_server_proc_run func, u32 ms)
{
	u8 i, count;

	for(i=0, count=0; i<DELAY_RUN_TASK_SIZE; i++)
	{
		if( g_delayRunTasks[i].func && 
			g_delayRunTasks[i].ms_count )
		{
			count++;
		}
	}

	g_delayRunTasks[index].func     = func;
	g_delayRunTasks[index].ms_count = ms;

	if(count==0) {
		TIM_Cmd(TIM5, ENABLE);
	}
}

void time_server_interrupt_proc5(void)
{
	u8 i, count;

	for(i=0, count=0; i<DELAY_RUN_TASK_SIZE; i++)
	{
		if( g_delayRunTasks[i].func && 
			g_delayRunTasks[i].ms_count )
		{
			g_delayRunTasks[i].ms_count--;
			if(g_delayRunTasks[i].ms_count==0) {
				g_delayRunTasks[i].func();
			}

			count++;
		}
	}

	if(count==0) {
		TIM_Cmd(TIM5, DISABLE);
	}
}

void SetUserTimeout(u32 nTimeout)
{
	g_nUserTimeout = nTimeout;
	get_system_timestamp(&g_userTimestamp);
}

u8 IsUserTimeout(void)
{
	return (get_system_relative_us_count(&g_userTimestamp)/1000 >g_nUserTimeout) ?1 :0;
}

void get_system_timestamp(system_timestamp *p_timestamp)
{
	g_isLockSysTimestamp = 1;

	p_timestamp->sec_count = g_sysTimestamp.sec_count;
	p_timestamp->us_count  = g_sysTimestamp.us_count;  

	g_isLockSysTimestamp = 0;
}

u32 get_system_relative_us_count(system_timestamp *p_timestamp)
{
	u32 nCount = 0;
	system_timestamp tmp_timestamp;

	get_system_timestamp(&tmp_timestamp);

	if( p_timestamp->sec_count > tmp_timestamp.sec_count ||
		(p_timestamp->sec_count == tmp_timestamp.sec_count && 
		 p_timestamp->us_count >= tmp_timestamp.us_count ))
	{
		return 0;
	}

	nCount = tmp_timestamp.sec_count - p_timestamp->sec_count;
	nCount = (nCount > 4000u) ?(4000000000u) :(nCount*1000000u);
	nCount = nCount + tmp_timestamp.us_count - p_timestamp->us_count;

	return nCount;
}

void system_delay_ms(u32 nms)
{
#if 0
	u32 nDelayUsCnt = nms * 1000;
	system_timestamp timestamp;

	get_system_timestamp(&timestamp);
	while(get_system_relative_us_count(&timestamp) < nDelayUsCnt);   
    
#else  
    
    u32 nDelayUsCnt = nms*10;
    
    while(nDelayUsCnt--)
        system_delay_100us(1);
#endif
}

void system_delay_10us(u32 n)
{
#if 0
	u32 nDelayUsCnt = n * 10;
	system_timestamp timestamp;

	get_system_timestamp(&timestamp);
	while(get_system_relative_us_count(&timestamp) < nDelayUsCnt);   
    
#else  
    
	u32 nDelayUsCnt = n*10;
    
    while(nDelayUsCnt--) {
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); 
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    }
#endif
}

void system_delay_100us(u32 n)
{
#if 0
	u32 nDelayUsCnt = n * 100;
	system_timestamp timestamp;

	get_system_timestamp(&timestamp);
	while(get_system_relative_us_count(&timestamp) < nDelayUsCnt);   
    
#else  
    
    u32 nDelayUsCnt = n*100;
    
    while(nDelayUsCnt--) {
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); 
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    }
#endif
}

void system_delay_us(u32 n)
{
	while (n--) 
	{
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); 
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
	}
}

void system_delay_500ns(void)
{
	//while(N500ns--)
	//{
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
	__NOP(); __NOP();
	//}
}
