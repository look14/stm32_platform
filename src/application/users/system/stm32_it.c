/**
  ******************************************************************************
  * @file    stm32_it.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/

#include "stm32_it.h"
#include "usb_istr.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "hw_config.h"

#include "stm32f10x_exti.h"
#include "Stm32f10x_tim.h"

#include "system.h"
#include "time_server.h"
#include "interrupt_proc_server.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                         */
/******************************************************************************/

/*******************************************************************************
* Function Name  : NMI_Handler
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
}

/*******************************************************************************
* Function Name  : HardFault_Handler
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : MemManage_Handler
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : BusFault_Handler
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : UsageFault_Handler
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : SVC_Handler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void)
{
}

/*******************************************************************************
* Function Name  : DebugMon_Handler
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void)
{
}

/*******************************************************************************
* Function Name  : PendSV_Handler
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void)
{
}

/******************************************************************************/
/*            STM32 Peripherals Interrupt Handlers                        */
/******************************************************************************/

/*******************************************************************************
* Function Name  : USB_HP_CAN1_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    USB_Istr();
}

/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Description    : This function handles DMA1 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{  
 
}

/*******************************************************************************
* Function Name  : USB_FS_WKUP_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line18);
}

/******************************************************************************/
/*                 STM32 Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32xxx.s).                                            */
/******************************************************************************/

/*******************************************************************************
* Function Name  : PPP_IRQHandler
* Description    : This function handles PPP interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void PPP_IRQHandler(void)
{
}*/

void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line0);
		if(g_int_proc_srv_run_exit0) {
			g_int_proc_srv_run_exit0();
		}
	}
}

void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line1);
		if(g_int_proc_srv_run_exit1) {
			g_int_proc_srv_run_exit1();
		}
	}
}

void EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line2);
		if(g_int_proc_srv_run_exit2) {
			g_int_proc_srv_run_exit2();
		}
	}
}

void EXTI3_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line3) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line3);
		if(g_int_proc_srv_run_exit3) {
			g_int_proc_srv_run_exit3();
		}
	}
}

void EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line4);
		if(g_int_proc_srv_run_exit4) {
			g_int_proc_srv_run_exit4();
		}
	}
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line5) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line5);
		if(g_int_proc_srv_run_exit5) {
			g_int_proc_srv_run_exit5();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line6) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line6);
		if(g_int_proc_srv_run_exit6) {
			g_int_proc_srv_run_exit6();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line7) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line7);
		if(g_int_proc_srv_run_exit7) {
			g_int_proc_srv_run_exit7();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line8) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line8);
		if(g_int_proc_srv_run_exit8) {
			g_int_proc_srv_run_exit8();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line9) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line9);
		if(g_int_proc_srv_run_exit9) {
			g_int_proc_srv_run_exit9();
		}
	}
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line10) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line10);
		if(g_int_proc_srv_run_exit10) {
			g_int_proc_srv_run_exit10();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line11) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line11);
		if(g_int_proc_srv_run_exit11) {
			g_int_proc_srv_run_exit11();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line12) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line12);
		if(g_int_proc_srv_run_exit12) {
			g_int_proc_srv_run_exit12();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line13) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line13);
		if(g_int_proc_srv_run_exit13) {
			g_int_proc_srv_run_exit13();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line14) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line14);
		if(g_int_proc_srv_run_exit14) {
			g_int_proc_srv_run_exit14();
		}
	}

	if(EXTI_GetITStatus(EXTI_Line15) != RESET) { 
		EXTI_ClearITPendingBit(EXTI_Line15);
		if(g_int_proc_srv_run_exit15) {
			g_int_proc_srv_run_exit15();
		}
	}
}

void TIM4_IRQHandler(void)   // if TIM4
{
	//if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	if((TIM4->SR & TIM_FLAG_Update) && (TIM4->DIER & TIM_FLAG_Update))
	{
		//TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);
        TIM4->SR = (uint16_t)~TIM_FLAG_Update;

		/*g_sysTimestamp.us_count += DEF_TIMESTAMP_US_STEP;
		if(!g_isLockSysTimestamp && g_sysTimestamp.us_count >= DEF_TIMESTAMP_US_SIZE)
		{
			g_sysTimestamp.us_count -= DEF_TIMESTAMP_US_SIZE;
			g_sysTimestamp.sec_count++;
		}*/
	} 
}

void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM5, TIM_FLAG_Update);

		time_server_interrupt_proc5();
	}
}

void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM6, TIM_FLAG_Update);

		if(g_time_server_proc_run6) {
			g_time_server_proc_run6();
		}
	}
}

void SysTick_Handler(void)
{
	//while (1)
	//	goto_faultCode();  //this be  instead by OS_CPU_SysTick_Handler
    
    g_sysTimestamp.us_count += DEF_TIMESTAMP_US_STEP;
    if(!g_isLockSysTimestamp && g_sysTimestamp.us_count >= DEF_TIMESTAMP_US_SIZE)
	{
		g_sysTimestamp.us_count -= DEF_TIMESTAMP_US_SIZE;
		g_sysTimestamp.sec_count++;
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
