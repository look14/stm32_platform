#include "system.h"
#include "usb_lib.h"

#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_tim.h"

#include "usb_pwr.h"

#include "time_server.h"
#include "gpio_defs.h"
#include "common.h"

RCC_ClocksTypeDef g_System_RCC_Clocks;

void system_config()
{
	Rcc_Config();
	Nvic_Config(); 
	Gpio_Config();
	//Exti_Config();
	SystemTimerDelay_Config();
	Timer5_Config();
	//Pwm_Config();

	USB_Interrupts_Config();
    Set_USBClock();    
    USB_Init();
}

void Gpio_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	SET_GPIO_L(USB_SOFT_CONNECT_GPIO);
	SET_GPIO_OUT(USB_SOFT_CONNECT_GPIO);
}

void Rcc_Config(void)
{
	/* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/   
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	if(RCC_WaitForHSEStartUp() == SUCCESS)   /* Wait till HSE is ready 等待外部晶振启动*/
	{
     /* Enable Prefetch Buffer */
     FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

     /* Flash 2 wait state */
     FLASH_SetLatency(FLASH_Latency_2);
    
     /* HCLK = SYSCLK */
     RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
     /* PCLK2 = HCLK */
     RCC_PCLK2Config(RCC_HCLK_Div1); 
 
     /* PCLK1 = HCLK/2 */
     RCC_PCLK1Config(RCC_HCLK_Div2);
 
     /* PLLCLK = 8MHz * 9 = 72 MHz */
     RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);		 //9倍频  8M*9=72M
 
     /* Enable PLL */ 
     RCC_PLLCmd(ENABLE);
 
     /* Wait till PLL is ready */
     while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
     {
     }
 
     /* Select PLL as system clock source */
     RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
 
     /* Wait till PLL is used as system clock source */
     while(RCC_GetSYSCLKSource() != 0x08)
     {
     }
   }
}

void Nvic_Config(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;

	#ifdef  VECT_TAB_RAM  										//向量表基地址选择
		NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);  			//将0x20000000地址作为向量表基地址(RAM)
	#else  
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); 			//将0x08000000地址作为向量表基地址(FLASH)
	#endif

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel 						= TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel             			= TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority  			= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd          			= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel             			= TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority  			= 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd          			= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel 						= USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= ENABLE;
} 

void Exti_Config(void)
{
	EXTI_InitTypeDef  EXTI_InitStructure;							//定义一个EXTI结构体变量

	/* Configure the EXTI line 18 connected internally to the USB IP */
/*	EXTI_ClearITPendingBit(EXTI_Line18);
	EXTI_InitStructure.EXTI_Line = EXTI_Line18; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);			*/

  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);	//配置端口C的13引脚为中断源	  重要！！ 板上标号INT2
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0); 	//配置端口E的0引脚为中断源	  重要！！ 板上标号INT1
	EXTI_InitStructure.EXTI_Line = EXTI_Line0 | EXTI_Line13;		//
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;				//中断模式为中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;			//下降沿出发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;						//使能中断线
	EXTI_Init(&EXTI_InitStructure);									//根据参数初始化中断寄存器
}

void SystemTimerDelay_Config(void)  //sample as 72  is 72Mhz ,us TIM4,1us tick
{
    SysTick_Config(72*DEF_TIMESTAMP_US_STEP);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}

void Timer5_Config()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
	TIM_DeInit(TIM5);
		
	TIM_TimeBaseStructure.TIM_Period		= 1000 - 1;				// 1ms 
	TIM_TimeBaseStructure.TIM_Prescaler		= 72 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision	= TIM_CKD_DIV1;    		// 采样分频
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;   // 向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
		
	TIM_ClearFlag(TIM5, TIM_FLAG_Update);       					// 清除溢出中断标志
	TIM_ARRPreloadConfig(TIM5, DISABLE);       						// 禁止ARR预装载缓冲器
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	 
	TIM_Cmd(TIM5, DISABLE);
}

void Timer6_Config(u32 nDataRate)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	TIM_DeInit(TIM6);

	nDataRate = (u32)1000000u / nDataRate;
		
	TIM_TimeBaseStructure.TIM_Period		= nDataRate - 1; 
	TIM_TimeBaseStructure.TIM_Prescaler		= 72 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision	= TIM_CKD_DIV1;    	 
	TIM_TimeBaseStructure.TIM_CounterMode	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
		
	TIM_ClearFlag(TIM6, TIM_FLAG_Update);       				
	TIM_ARRPreloadConfig(TIM6, DISABLE);       					
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	 
	TIM_Cmd(TIM6, DISABLE);
}

void Pwm_Config(u16 Dutyfactor)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;					//定义一个通道输出结构

	TIM_OCStructInit(&TIM_OCInitStructure);					//设置缺省值
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	   	//PWM 模式 1 输出 	
	TIM_OCInitStructure.TIM_Pulse = Dutyfactor; 			//设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
															//PWM的输出频率为Fpwm=72M/7200=1Mhz； 
															 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	//TIM 输出比较极性高   	    
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;    //使能输出状态  需要PWM输出才需要这行

    TIM_OC2Init(TIM4, &TIM_OCInitStructure);				//根据参数初始化PWM寄存器    
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);	   	//使能 TIMx在 CCR2 上的预装载寄存器

	TIM_OC1Init(TIM4, &TIM_OCInitStructure);				//根据参数初始化PWM寄存器    
	TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);	   	//使能 TIMx在 CCR1 上的预装载寄存器

    TIM_CtrlPWMOutputs(TIM4,ENABLE);  						//设置TIM4 的PWM 输出为使能  
}

//不能在这里执行所有外设复位!否则至少引起串口不工作.
//把所有时钟寄存器复位
//CHECK OK
//091209
void SYSTEM_MYRCC_DeInit(void)
{
	RCC->APB1RSTR = 0x00000000; //复位结束
	RCC->APB2RSTR = 0x00000000;

	RCC->AHBENR = 0x00000014;       //flash时钟,闪存时钟使能.DMA时钟关闭
	RCC->APB2ENR = 0x00000000;      //外设时钟关闭.
	RCC->APB1ENR = 0x00000000;
	RCC->CR |= 0x00000001;          //使能内部高速时钟HSION
	RCC->CFGR &= 0xF8FF0000;        //复位SW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]
	RCC->CR &= 0xFEF6FFFF;          //复位HSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;          //复位HSEBYP
	RCC->CFGR &= 0xFF80FFFF;        //复位PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE
	RCC->CIR = 0x00000000;          //关闭所有中断
	//配置向量表
}

//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI
//CHECK OK
//091209
__asm void SYSTEM_WFI_SET(void)
{
	WFI;
}

//进入待机模式
//check ok
//091202
void SYSTEM_Standby(void)
{
	SCB->SCR |= 1 << 2;             //使能SLEEPDEEP位 (SYS->CTRL)
	RCC->APB1ENR |= 1 << 28;        //使能电源时钟
	RCC->APB1ENR |= 1 << 27;        //使能备份时钟
	PWR->CSR |= 1 << 8;             //设置WKUP用于唤醒
	PWR->CR |= 1 << 2;              //清除Wake-up 标志
	PWR->CR |= 1 << 1;              //PDDS置位
	SYSTEM_WFI_SET();               //执行WFI指令
	//__WFI();
}

//系统软复位
//CHECK OK
//091209
void SYSTEM_Soft_Reset(void)
{
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}

void SYSTEM_Get_sysClockFreq(void)
{
	RCC_GetClocksFreq(&g_System_RCC_Clocks);
}
