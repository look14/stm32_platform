#ifndef __GPIO_DEFS_H
#define __GPIO_DEFS_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
 
#define USB_SOFT_CONNECT_GPIO 		    GPIOA
#define USB_SOFT_CONNECT_GPIO_PIN	    GPIO_Pin_12
 
#define UART_TX_ICMODE_GPIO  		    GPIOA
#define UART_TX_ICMODE_GPIO_PIN 	    GPIO_Pin_2

#define UART_RX_ICMODE_GPIO  		    GPIOA
#define UART_RX_ICMODE_GPIO_PIN		    GPIO_Pin_3

#define KEY1_GPIO						GPIOE
#define KEY1_GPIO_PIN					GPIO_Pin_0

#define KEY2_GPIO						GPIOC
#define KEY2_GPIO_PIN					GPIO_Pin_13

#define LED1_GPIO                       GPIOD
#define LED1_GPIO_PIN			        GPIO_Pin_13

#define LED2_GPIO   			        GPIOG
#define LED2_GPIO_PIN			        GPIO_Pin_14

#define MPU6050_SCK_GPIO                GPIOE
#define MPU6050_SCK_GPIO_PIN            GPIO_Pin_1

#define MPU6050_SDA_GPIO                GPIOE
#define MPU6050_SDA_GPIO_PIN            GPIO_Pin_0


#define SET_GPIO_OUT(x)				GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_PP)
#define SET_GPIO_IN(x)				GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING)
#define SET_GPIO_OD(x)				GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_Out_OD)
#define SET_GPIO_AIN(x)				GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_AIN)
#define SET_GPIO_AFOUT(x)			GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_AF_PP)
#define SET_GPIO_AFOD(x)			GPIO_Pin_Setting(x, x##_PIN, GPIO_Speed_50MHz, GPIO_Mode_AF_OD)
#define SET_GPIO_H(x)				(x->BSRR = x##_PIN) //GPIO_SetBits(x, x##_PIN)
#define SET_GPIO_L(x)				(x->BRR  = x##_PIN) //GPIO_ResetBits(x, x##_PIN)
#define READ_GPIO_PIN(x)			(((x->IDR & x##_PIN)!=Bit_RESET) ?1 :0) //GPIO_ReadInputDataBit(x, x##_PIN) 

#endif
