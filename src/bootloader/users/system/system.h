#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "typedefs.h"
#include "stm32f10x_rcc.h"

extern RCC_ClocksTypeDef g_System_RCC_Clocks;

void system_config(void);

void Rcc_Config(void);
void Nvic_Config(void);
void Exti_Config(void);
void Gpio_Config(void);
void SystemTimerDelay_Config(void);
void Timer5_Config(void);
void Timer6_Config(u32 nDataRate);
void Pwm_Config(u16 Dutyfactor);

void SYSTEM_MYRCC_DeInit(void);
void SYSTEM_Standby(void);
void SYSTEM_Get_sysClockFreq(void);
void SYSTEM_Soft_Reset(void);

#endif
