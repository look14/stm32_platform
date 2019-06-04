#ifndef __COMMON_H
#define __COMMON_H

#include "typedefs.h"
#include "stm32f10x_gpio.h"

#include "i2c.h"

#define DEF_STM32HL_FLASH_BASE 			(u32)0x08000000
#define DEF_STM32HL_FLASH_PAGE_SIZE 	(u32)2048

#define DEF_BOOTLOADER_START_ADDR  		DEF_STM32HL_FLASH_BASE
#define DEF_BOOTLOADER_SIZE 			(u32)0x4000

#define DEF_APP_CODE_START_ADDR         (u32)(DEF_BOOTLOADER_SIZE)                           	// 应用程序开始的位置
#define DEF_APP_CODE_SIZE             	(u32)(DEF_BURNER_FLASH_OFFSET - DEF_BOOTLOADER_SIZE)    // 应用程序空间最大长度

#define DEF_APP_VALIDE_FLAG_START    	(u32)0x08040000                                         // 升级标志的位置//at 128kbytes	

#define DEF_SYSTEM_SETTINGS_ADDR		(u32)0x08040800
#define DEF_SYSTEM_SETTINGS_SIZE		sizeof(g_system_settings)

#define USB_BUF_SIZE	64

extern u8 g_bUsbSendReady;
extern u8 g_bUsbRecvReady;

extern u8 g_usbSendBuf[USB_BUF_SIZE];
extern u8 g_usbRecvBuf[USB_BUF_SIZE];

extern u8 g_nStm32SoftResetFlag;

extern i2c_type g_i2c_mpu6050;

void common_init(void);

void set_u16_to_buf(u8 buf[], u16 dat16);
u16 get_u16_from_buf(const u8 buf[]);

void set_u32_to_buf(u8 buf[], u32 dat32);
u32 get_u32_from_buf(const u8 buf[]);

void no_optimize(const void* p_param);
void GPIO_Pin_Setting(GPIO_TypeDef *gpio, uint16_t nPin, GPIOSpeed_TypeDef speed, GPIOMode_TypeDef mode);
void Gpio_Config(void);

u8 flash_erase(u32 dwAdd, u32 dwLen);
u8 flash_write(u32 dwAdd, const u8 *pWrBuf, u32 dwLen);
u8 flash_read(u32 dwAdd, u8 *pRdBuf, u32 dwLen);

#endif
