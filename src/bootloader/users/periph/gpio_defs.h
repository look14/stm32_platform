#ifndef __GPIO_DEFS_H
#define __GPIO_DEFS_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
 
#define USB_SOFT_CONNECT_GPIO 		GPIOA
#define USB_SOFT_CONNECT_GPIO_PIN	GPIO_Pin_12
 
#define EVB_UART_TX_ICMODE_GPIO  	GPIOA
#define EVB_UART_TX_ICMODE_GPIO_PIN GPIO_Pin_2

#define EVB_UART_RX_ICMODE_GPIO  	GPIOA
#define EVB_UART_RX_ICMODE_GPIO_PIN	GPIO_Pin_3


#define SET_GPIO_OUT(x)				GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define SET_GPIO_IN(x)				GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
#define SET_GPIO_OD(x)				GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_OD)
#define SET_GPIO_H(x)				(x->BSRR = x##_PIN) //GPIO_SetBits(x, x##_PIN)
#define SET_GPIO_L(x)				(x->BRR  = x##_PIN) //GPIO_ResetBits(x, x##_PIN)
#define READ_GPIO_PIN(x)			(((x->IDR & x##_PIN)!=Bit_RESET) ?1 :0) //GPIO_ReadInputDataBit(x, x##_PIN) 

#endif
