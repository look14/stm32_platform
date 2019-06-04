#ifndef __STM32_H
#define __STM32_H

#include "typedefs.h"

#define DEF_STM32HL_FLASH_BASE 			(u32)0x08000000
#define DEF_STM32HL_FLASH_PAGE_SIZE 	(u32)2048

#define DEF_BOOTLOADER_START_ADDR  		DEF_STM32HL_FLASH_BASE
#define DEF_BOOTLOADER_SIZE 			(u32)0x4000

#define DEF_APP_CODE_START_ADDR         (u32)(DEF_BOOTLOADER_SIZE)    // 应用程序开始的位置
#define DEF_APP_VALIDE_FLAG_START    	(u32)0x08040000               // 升级标志的位置//at 128kbytes	
#define DEF_SYSTEM_SETTINGS_ADDR		(u32)0x08040800

class Stm32 {
	
public:
	Stm32();
	virtual ~Stm32();
	
	u8 Ping();
	u8 GetFirmware(char* strFirmware);
	u8 GetBootFirmware(u32 *pVersion);
	u8 GetAppFirmware(u32 *pVersion);
	u8 SoftReset();

	u8 LockUsb();
	u8 UnlockUsb();

	u8 ClearUpdateFlag();
	u8 WriteUpdateFlag();
	u8 CheckUpdateFlag();

	u8 ReadHexFile(const char* strFile, u32 nRomBaseAddr, u32 *pRomLen, u8 arrRomBuf[]);
	u8 UpgradeAppCode(const char* strFile);
	double GetUpgradeProgress() { return m_fUpgradeProgress; }

	static u8 ReadFlash(u32 addr, u8 buf[], u32 len);
	static u8 WriteFlash(u32 addr, const u8 buf[], u32 len);

	static u8 ServerConfig_ReadFlash(u32 addr, u8 buf[], u32 len);
	static u8 ServerConfig_WriteFlash(u32 addr, const u8 buf[], u32 len);
protected:
	double m_fUpgradeProgress;
};



#endif
