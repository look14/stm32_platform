/**
  ******************************************************************************
  * @file    platform_config.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Evaluation board specific configuration file.
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line corresponding to the STMicroelectronics evaluation board
   used to run the example */
#if !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL)
 //#define USE_STM3210B_EVAL
 //#define USE_STM3210E_EVAL
 #define USE_STM3210X_EVAL
#endif


/* Define the STM32F10x hardware depending on the used evaluation board */
#ifdef USE_STM3210B_EVAL
  #define USB_DISCONNECT                      GPIOD  
  #define USB_DISCONNECT_PIN                  GPIO_Pin_9
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD

  #define RCC_APB2Periph_GPIO_KEY             RCC_APB2Periph_GPIOB
  #define RCC_APB2Periph_GPIO_TAMPER          RCC_APB2Periph_GPIOC
  #define RCC_APB2Periph_GPIO_IOAIN           RCC_APB2Periph_GPIOC
	//ST link 32EVAL
	//#ifdef 1
  #define RCC_APB2Periph_GPIO_LED             RCC_APB2Periph_GPIOB

  #define GPIO_KEY                            GPIOB
  #define GPIO_TAMPER                         GPIOC
  #define GPIO_IOAIN                          GPIOC
  #define GPIO_LED                            GPIOB

  #define GPIO_KEY_PIN                        GPIO_Pin_9   /* PB.09 */
  #define GPIO_TAMPER_PIN                     GPIO_Pin_13  /* PC.13 */
  #define GPIO_IOAIN_PIN                      GPIO_Pin_4   /* PC.04 */
  
  #define GPIO_LED1_PIN                       GPIO_Pin_5   /* PC.06 */
  #define GPIO_LED2_PIN                       GPIO_Pin_6   /* PC.07 */ 
  #define GPIO_LED3_PIN                       GPIO_Pin_7   /* PC.08 */
  #define GPIO_LED4_PIN                       GPIO_Pin_8   /* PC.09 */ 
//#endif

//STM32 MB525B


  #define GPIO_KEY_PORTSOURCE                 GPIO_PortSourceGPIOB
  #define GPIO_KEY_PINSOURCE                  GPIO_PinSource9
  #define GPIO_KEY_EXTI_Line                  EXTI_Line9

  #define GPIO_TAMPER_PORTSOURCE              GPIO_PortSourceGPIOC
  #define GPIO_TAMPER_PINSOURCE               GPIO_PinSource13
  #define GPIO_TAMPER_EXTI_Line               EXTI_Line13

  #define ADC_AIN_CHANNEL                     ADC_Channel_14

#elif defined(USE_STM3210E_EVAL)
  #define USB_DISCONNECT                      GPIOB  
  #define USB_DISCONNECT_PIN                  GPIO_Pin_14
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOB

  #define RCC_APB2Periph_GPIO_KEY             RCC_APB2Periph_GPIOG
  #define RCC_APB2Periph_GPIO_TAMPER          RCC_APB2Periph_GPIOC
  #define RCC_APB2Periph_GPIO_IOAIN           RCC_APB2Periph_GPIOC
 #define RCC_APB2Periph_GPIO_LED             RCC_APB2Periph_GPIOF
	

  #define GPIO_KEY                            GPIOG
  #define GPIO_TAMPER                         GPIOC
  #define GPIO_IOAIN                          GPIOC
  #define GPIO_LED                            GPIOF

  #define GPIO_KEY_PIN                        GPIO_Pin_8   /* PG.08 */
  #define GPIO_TAMPER_PIN                     GPIO_Pin_13  /* PC.13 */
  #define GPIO_IOAIN_PIN                      GPIO_Pin_4   /* PC.04 */
  
  #define GPIO_LED1_PIN                       GPIO_Pin_6   /* PF.06 */
  #define GPIO_LED2_PIN                       GPIO_Pin_7   /* PF.07 */ 
  #define GPIO_LED3_PIN                       GPIO_Pin_8   /* PF.08 */
  #define GPIO_LED4_PIN                       GPIO_Pin_9   /* PF.09 */ 

  #define GPIO_KEY_PORTSOURCE                 GPIO_PortSourceGPIOG
  #define GPIO_KEY_PINSOURCE                  GPIO_PinSource8
  #define GPIO_KEY_EXTI_Line                  EXTI_Line8

  #define GPIO_TAMPER_PORTSOURCE              GPIO_PortSourceGPIOC
  #define GPIO_TAMPER_PINSOURCE               GPIO_PinSource13
  #define GPIO_TAMPER_EXTI_Line               EXTI_Line13

  #define ADC_AIN_CHANNEL                     ADC_Channel_14
#elif defined(USE_STM3210X_EVAL)
  #define USB_DISCONNECT                      GPIOB  
  #define USB_DISCONNECT_PIN                  GPIO_Pin_15
  #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOB

  #define RCC_APB2Periph_GPIO_KEY             RCC_APB2Periph_GPIOG
  #define RCC_APB2Periph_GPIO_TAMPER          RCC_APB2Periph_GPIOC
  #define RCC_APB2Periph_GPIO_IOAIN           RCC_APB2Periph_GPIOC
  #define RCC_APB2Periph_GPIO_LED             RCC_APB2Periph_GPIOD
	

  #define GPIO_KEY                            GPIOG
  #define GPIO_TAMPER                         GPIOC
  #define GPIO_IOAIN                          GPIOC
  #define GPIO_LED                            GPIOD

  #define GPIO_KEY_PIN                        GPIO_Pin_9   /* PB.09 */
  #define GPIO_TAMPER_PIN                     GPIO_Pin_13  /* PC.13 */
  #define GPIO_IOAIN_PIN                      GPIO_Pin_4   /* PC.04 */
  
  #define GPIO_LED1_PIN                       GPIO_Pin_2   /* PF.06 */
  #define GPIO_LED2_PIN                       GPIO_Pin_3   /* PF.07 */ 
  #define GPIO_LED3_PIN                       GPIO_Pin_4   /* PF.08 */
  #define GPIO_LED4_PIN                       GPIO_Pin_7   /* PF.09 */ 

  #define GPIO_KEY_PORTSOURCE                 GPIO_PortSourceGPIOG
  #define GPIO_KEY_PINSOURCE                  GPIO_PinSource8
  #define GPIO_KEY_EXTI_Line                  EXTI_Line8

  #define GPIO_TAMPER_PORTSOURCE              GPIO_PortSourceGPIOC
  #define GPIO_TAMPER_PINSOURCE               GPIO_PinSource13
  #define GPIO_TAMPER_EXTI_Line               EXTI_Line13

  #define ADC_AIN_CHANNEL                     ADC_Channel_14
#endif /* USE_STM3210B_EVAL */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __PLATFORM_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

