#include "typedefs.h"
#include "stm32f10x_dbgmcu.h"

#include "boot_loader.h"
#include "system.h"
#include "common.h"

#include "task.h"

int main(void)  
{
	SystemInit(); // if not-ucos
	SYSTEM_Get_sysClockFreq();

	DBGMCU_Config(DBGMCU_TIM1_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_TIM2_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_TIM3_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_TIM4_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_TIM5_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_TIM6_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_TIM7_STOP, ENABLE);
	DBGMCU_Config(DBGMCU_TIM8_STOP, ENABLE);

	task_init();
	task_run();
}

