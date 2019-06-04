#include "typedefs.h"
#include "stm32f10x_dbgmcu.h"

#include "boot_loader.h"
#include "system.h"
#include "common.h"

#include "task.h"

volatile const u8 g_Invalide_APP_Flag_InBootloader[16] __attribute__((at(DEF_APP_VALIDE_FLAG_START)));

int main(void)  
{
#if 0
	Bootloader_flag_erase();    
	Bootloader_flag_write(); 
#endif

	if (Bootloader_Check_App_FW_Validity() == MODE_APP_FW_VALID) {
		Bootloader_closeBsp();								
		Flash_JumpToExeCode();
	} 
	else {
	}

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

