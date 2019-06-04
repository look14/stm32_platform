#include "boot_loader.h"
#include "system.h"
#include "common.h"
#include "stm32f10x_tim.h"

const u8 g_APP_Flag[] = { 0x33, 0xCC, 0x55, 0xAA, 0x00, 0xFF, 0x11, 0xEE, 'l', 'o', 'o', 'k', ' ', '1', '3' };  //����������ֵ
#define DEF_APP_VALIDE_FLAG_LEN  sizeof(g_APP_Flag)

u8 Bootloader_Check_App_FW_Validity()
{
	u16 i = 0;
	u8 bLen = DEF_APP_VALIDE_FLAG_LEN;
	u8 *pAPP_Flag_Flash = (u8 *)DEF_APP_VALIDE_FLAG_START;

	//----------------------------------
	// update application program
	//----------------------------------
	for (i = 0; i < bLen; i++)
	{
		if (pAPP_Flag_Flash[i] != g_APP_Flag[i])
			return MODE_APP_FW_INVALID;
	}

	return MODE_APP_FW_VALID;
}


void Bootloader_App_erase()
{
	flash_erase((u32)DEF_STM32HL_FLASH_BASE + DEF_APP_CODE_START_ADDR, DEF_STM32HL_FLASH_PAGE_SIZE);
}

void Bootloader_flag_erase()
{
	flash_erase((u32)DEF_APP_VALIDE_FLAG_START, DEF_STM32HL_FLASH_PAGE_SIZE);
}

void Bootloader_flag_write()
{
	u8 bLen = DEF_APP_VALIDE_FLAG_LEN;

	flash_write(DEF_APP_VALIDE_FLAG_START, (u8 *)g_APP_Flag, bLen);
}

void Bootloader_closeBsp()
{
	SYSTEM_MYRCC_DeInit();

	TIM_Cmd(TIM1, DISABLE);
	TIM_Cmd(TIM2, DISABLE);
	TIM_Cmd(TIM3, DISABLE);
	TIM_Cmd(TIM4, DISABLE);
	TIM_Cmd(TIM5, DISABLE);
	TIM_Cmd(TIM6, DISABLE);
	TIM_Cmd(TIM7, DISABLE);
	TIM_Cmd(TIM8, DISABLE);	
}

u8 Flash_JumpToExeCode()
{
	//typedef void (*pCallFunction)(void);
	//pCallFunction Call_UserApp = 0;

	//stm32 bin code: ��һ��U32 ��R13(SP)��ֵ���ڶ�����Reset_Handler����ת��ַ���������Ǵ���ĵ�ַ

	u32 tmpDEF_CODE_START = DEF_STM32HL_FLASH_BASE + DEF_APP_CODE_START_ADDR ;  //    (U32)0x08004000
	//u32 dwStm32JumpAddress;

	if (((*(volatile unsigned long *)tmpDEF_CODE_START) & 0x2FFE0000) != 0x20000000) {             //������ܳɹ��Ļ�����Ӧ�ó���ı�־�����ʹ�����bootloader��
		Bootloader_flag_erase();
		tmpDEF_CODE_START = DEF_STM32HL_FLASH_BASE; 
	}

	//��ת���û�����
	//dwStm32JumpAddress = *(volatile unsigned long *)((tmpDEF_CODE_START) + 4);            //���û�����ĸ�λ��ַ��dwStm32JumpAddress
	//Call_UserApp = (pCallFunction)dwStm32JumpAddress;

	//��ʼ���û�����Ķ�ջָ��
	__set_MSP(*(volatile unsigned long *)(tmpDEF_CODE_START));                  //���û�����Ķ�ջ��ַд���ջָ��
	// �����������˶�ջָ��֮�󣬲�����ת

	//Call_UserApp();
	SYSTEM_Soft_Reset(); // disable all irq

	return 0;
}
