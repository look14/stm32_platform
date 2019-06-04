#include <string.h>
#include "common.h"

#include "stm32f10x_flash.h"
#include "platform_config.h"

u8 g_bUsbSendReady;
u8 g_bUsbRecvReady;

u8 g_usbSendBuf[USB_BUF_SIZE];
u8 g_usbRecvBuf[USB_BUF_SIZE];

u8 g_nStm32SoftResetFlag;

i2c_type g_i2c_mpu6050;

void common_init(void)
{
	g_bUsbSendReady = 0;
	g_bUsbRecvReady = 0;

	memset(g_usbSendBuf, 0, sizeof(g_usbSendBuf));
	memset(g_usbRecvBuf, 0, sizeof(g_usbRecvBuf));

	g_nStm32SoftResetFlag = 0;
}

void set_u16_to_buf(u8 buf[], u16 dat16)
{
	buf[0] = (u8)dat16;
	buf[1] = (u8)(dat16 >> 8);
}

u16 get_u16_from_buf(const u8 buf[])
{
	u16 dat16 = 0;
	dat16  = buf[0];
	dat16 |= ((u16)buf[1]) << 8;
	return dat16;
}

void set_u32_to_buf(u8 buf[], u32 dat32)
{
	buf[0] = (u8)dat32;
	buf[1] = (u8)(dat32 >> 8);
	buf[2] = (u8)(dat32 >> 16);
	buf[3] = (u8)(dat32 >> 24);
}

u32 get_u32_from_buf(const u8 buf[])
{
	u32 dat32 = 0;
	dat32  = buf[0];
	dat32 |= ((u32)buf[1]) << 8;
	dat32 |= ((u32)buf[2]) << 16;
	dat32 |= ((u32)buf[3]) << 24;
	return dat32;
}

void no_optimize(const void* p_param)
{
}

void GPIO_Pin_Setting(GPIO_TypeDef *gpio, uint16_t nPin, GPIOSpeed_TypeDef speed, GPIOMode_TypeDef mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = nPin;
	GPIO_InitStructure.GPIO_Speed = speed;
	GPIO_InitStructure.GPIO_Mode = mode;
	GPIO_Init(gpio, &GPIO_InitStructure);
}

void Gpio_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	SET_GPIO_L(USB_SOFT_CONNECT_GPIO);
	SET_GPIO_OUT(USB_SOFT_CONNECT_GPIO);
    
	SET_GPIO_IN(KEY1_GPIO);
    SET_GPIO_IN(KEY2_GPIO);
    
    g_i2c_mpu6050.sck_type = MPU6050_SCK_GPIO;
	g_i2c_mpu6050.sda_type = MPU6050_SDA_GPIO;
	g_i2c_mpu6050.sck_pin  = MPU6050_SCK_GPIO_PIN;
	g_i2c_mpu6050.sda_pin  = MPU6050_SDA_GPIO_PIN;
	g_i2c_mpu6050.delay    = 0;
	g_i2c_mpu6050.dev_addr = 0xD0;
}

typedef enum { FAILED = 0, PASSED = !FAILED } TestStatus;
volatile FLASH_Status g_FLASHStatus = FLASH_COMPLETE;
volatile TestStatus g_MemoryProgramStatus = PASSED;

u8 flash_erase(u32 dwAdd, u32 dwLen)
{
//dwLen��byte len,Ҫ��������4�ı���,ByteLen
	int i;
	int nPages = dwLen / DEF_STM32HL_FLASH_PAGE_SIZE;

	if (dwLen % DEF_STM32HL_FLASH_PAGE_SIZE)
		nPages += 1;

	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	FLASH_Unlock();                            //  /* Flash unlock */  //����FLASH����������ƼĴ�

	for (i = 0; i < nPages; i++) {
		g_FLASHStatus = FLASH_ErasePage(dwAdd);                   //����FLASH
		dwAdd += DEF_STM32HL_FLASH_PAGE_SIZE;
	}
	FLASH_Lock();

	return 0;
}

u8 flash_write(u32 dwAdd, const u8 *pWrBuf, u32 dwLen)
{
//dwLen��byte len,Ҫ��������4�ı���,ByteLen
	u32 i = 0;
    u32 dat32;

	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	FLASH_Unlock();
	for (i = 0; i < dwLen; i += 4) {
		dat32 = get_u32_from_buf(&pWrBuf[i]);
		g_FLASHStatus = FLASH_ProgramWord(i+dwAdd, dat32);
	}
	FLASH_Lock();

	return 0;
}

u8 flash_read(u32 dwAdd, u8 *pRdBuf, u32 dwLen)
{
	//dwLenҪ��������4�ı���
	memcpy(pRdBuf, (u8 *)dwAdd, dwLen);

	return 0;
}