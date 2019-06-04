#include "stdafx.h"
#include "stm32.h"
#include "server_comm_control.h"
#include "server_config.h"

#include <fstream>
#include <string>
#include <algorithm>
using namespace std;

const u8 g_APP_Flag[] = { 0x33, 0xCC, 0x55, 0xAA, 0x00, 0xFF, 0x11, 0xEE, 'l', 'o', 'o', 'k', ' ', '1', '3' };
#define DEF_APP_VALIDE_FLAG_LEN  sizeof(g_APP_Flag)

Stm32::Stm32()
{
	m_fUpgradeProgress = 0;
}

Stm32::~Stm32()
{
	
}

u8 Stm32::Ping()
{
	return CommControl::GetInstance()->Ping();
}

u8 Stm32::GetFirmware(char* strFirmware)
{
	u8 nRet;
	u16 len;

	nRet = CommControl::GetInstance()->GetFirmware((u8*)strFirmware, &len);
	return nRet;
}

u8 Stm32::GetBootFirmware(u32 *pVersion)
{
	char strVer[256];
	u8 nRet;
	u16 len;
	
	nRet = CommControl::GetInstance()->GetFirmware((u8*)strVer, &len);
	if(nRet==0)
	{
		if(0==sscanf(strVer, "boot.%lu", pVersion))
			nRet = 1;
	}
	
	return nRet;
}

u8 Stm32::GetAppFirmware(u32 *pVersion)
{
	char strVer[256];
	u8 nRet;
	u16 len;

	nRet = CommControl::GetInstance()->GetFirmware((u8*)strVer, &len);
	if(nRet==0)
	{
		if(0==sscanf(strVer, "app.%lu", pVersion))
			nRet = 1;
	}

	return nRet;
}

u8 Stm32::SoftReset()
{
	u8 nRet;
	
	nRet = CommControl::GetInstance()->SoftReset();
	CommControl::GetInstance()->CloseUsb();

	if(nRet == 0) {
		clock_t clk_beg = clock(); 

		nRet = COMM_CONTROL_ERR_COMM;

		while(clock() - clk_beg < 15000) 
		{
			if(CommControl::GetInstance()->IsUsbReady()) {
				nRet = COMM_CONTROL_ERR_OK;
				break;
			}
			Sleep(50);
		}
	}


	return nRet;
}

u8 Stm32::LockUsb()
{
	return CommControl::GetInstance()->EnterUsb();
}

u8 Stm32::UnlockUsb()
{
	return CommControl::GetInstance()->LeaveUsb();
}

u8 Stm32::ClearUpdateFlag()
{
	return CommControl::GetInstance()->Stm32EraseFlash(DEF_APP_VALIDE_FLAG_START, DEF_STM32HL_FLASH_PAGE_SIZE);
}

u8 Stm32::WriteUpdateFlag()
{
	return CommControl::GetInstance()->Stm32WriteFlash(DEF_APP_VALIDE_FLAG_START, g_APP_Flag, DEF_APP_VALIDE_FLAG_LEN);	
}

u8 Stm32::CheckUpdateFlag()
{
	u8 buf[32];
	u8 nRet;

	nRet = CommControl::GetInstance()->Stm32ReadFlash(DEF_APP_VALIDE_FLAG_START, buf, DEF_APP_VALIDE_FLAG_LEN);
	if(nRet==0)
	{
		nRet = memcmp(g_APP_Flag, buf, DEF_APP_VALIDE_FLAG_LEN);
	}

	return nRet;
}

u8 Stm32::ReadHexFile(const char* strFile, u32 nRomBaseAddr, u32 *pRomLen, u8 arrRomBuf[])
{
	ifstream in(strFile);
	if(!in)
		return 1;

	u8 nRet = 0;
	u32 i, index;

	//const char* p_str;
	string strLine;
	u8 hex_len;
	u32 hex_addr;
	u8 hex_type;
	u8  hex_buf[32];
	u8 hex_checksum, tmp_checksum = 0;
	u32 hex_page_addr = nRomBaseAddr;

	*pRomLen = 0;

	while(!in.eof())
	{
		getline(in, strLine);

		strLine.erase(0, strLine.find_first_not_of(": \r\n")); 
		strLine.erase(strLine.find_last_not_of(": \r\n") + 1);
        transform(strLine.begin(), strLine.end(), strLine.begin(), ::toupper);

		if(strLine.empty())
			continue;

		//p_str = strLine.c_str();
		//sscanf(&p_str[0], "%02X", &hex_len);
        hex_len = strtoul(strLine.substr(0,2).c_str(), NULL, 16);

		if(strLine.length() < (10 + 2*hex_len))
			continue;	// file error.

		//sscanf(&p_str[0], "%02X%04X%02X", &hex_len, &hex_addr, &hex_type);
		//sscanf(&p_str[8 + 2*hex_len], "%02X", &hex_checksum);
        hex_addr = strtoul(strLine.substr(2,4).c_str(), NULL, 16);
        hex_type = strtoul(strLine.substr(6,2).c_str(), NULL, 16);
        hex_checksum = strtoul(strLine.substr(8 + 2*hex_len,2).c_str(), NULL, 16);

		for(i=0; i<hex_len; i++) {
			//sscanf(&p_str[8+2*i], "%02X", &hex_buf[i]);
            hex_buf[i] = strtoul(strLine.substr(8+2*i,2).c_str(), NULL, 16);
		}

#if 1
		/////////////// check sum calculator ///////////////
		tmp_checksum  = 0;
		tmp_checksum += hex_len;
		tmp_checksum += hex_addr;
        tmp_checksum += hex_addr>>8;
		tmp_checksum += hex_type;
		for(i=0; i<hex_len; i++) {
			tmp_checksum += hex_buf[i];
		}
		tmp_checksum = ~tmp_checksum + 1;
		////////////////////////////////////////////////////

		if(tmp_checksum != hex_checksum) {
			nRet = 2;
			break;
		}
#endif

		if(hex_type == 0x04) {
			hex_page_addr   = hex_buf[0];
			hex_page_addr <<= 8;
			hex_page_addr  |= hex_buf[1];
			hex_page_addr <<= 16;
		}

		else if(hex_type == 0x00) {
			index = hex_page_addr + hex_addr - nRomBaseAddr;
			memcpy(&arrRomBuf[index], hex_buf, hex_len);
			*pRomLen = index + hex_len;
		}

		else if(hex_type == 0x05) {
			// program start addr
		}

		else if(hex_type == 0x01) {
			break;	// end hex
		}

		else {
			nRet = 3;
			break;
		}
	}
    
    if(*pRomLen==0)
        nRet = 4;

	in.close();

	return nRet;
}

u8 Stm32::UpgradeAppCode(const char* strFile)
{
	u8 nRet = 0;
	u32 nVersion;
	u32 nRomLen;
	u32 nRomBaseAddr = DEF_STM32HL_FLASH_BASE;
	const u32 DEF_TEMP_BUF_LEN = (u32)10 * 1024 * 1024;

	const u32 nUsbOnceLen = 256;

	u32 i, nLastPrintCnt;
	u32 nOnceLen, nAllLen, nStartAddr;
	
	char strMsg[256];
	u8 arrReadBuf[512];
	u8 arrWriteBuf[512];
	u8 *arrRomBuf = new u8[DEF_TEMP_BUF_LEN];
	memset(arrRomBuf, 0, DEF_TEMP_BUF_LEN);
	m_fUpgradeProgress = 0;

	nRet = ReadHexFile(strFile, nRomBaseAddr, &nRomLen, arrRomBuf);
	if(nRet !=0 )
		goto END;

	nRet = GetAppFirmware(&nVersion);
	if(nRet == 0 )
	{
		printf("app.%03lu\n", nVersion);

		nRet = CommControl::GetInstance()->EnterUsb();
		if(nRet !=0 )
			goto END;

		nRet = ClearUpdateFlag();
		if(nRet !=0 )
			goto END;
		
		SoftReset();
	}

	nRet = GetBootFirmware(&nVersion);
	if(nRet !=0 )
		goto END;

	printf("boot.%03lu\n", nVersion);

	nStartAddr = nRomBaseAddr + DEF_APP_CODE_START_ADDR;
	nAllLen    = nRomLen - DEF_APP_CODE_START_ADDR;

	nLastPrintCnt = 0;

	for(i=0; i<nAllLen; i += DEF_STM32HL_FLASH_PAGE_SIZE)
	{
		nOnceLen = __min(DEF_STM32HL_FLASH_PAGE_SIZE, nAllLen-i);
		
		nRet = CommControl::GetInstance()->Stm32EraseFlash(nStartAddr+i, nOnceLen);
		if(nRet != 0)
			break;

		m_fUpgradeProgress  = nOnceLen + i;
		m_fUpgradeProgress /= 1.0*nAllLen;

		while(nLastPrintCnt--)
			putchar('\b');
		
		sprintf(strMsg, "erase flash: %.2f%s", m_fUpgradeProgress*100, "%");
		printf("%s", strMsg);
        fflush (stdout);
		nLastPrintCnt = strlen(strMsg);
	}
	printf("\n");
	nLastPrintCnt = 0;

	for(i=0; i<nAllLen; i += nUsbOnceLen)
	{
		nOnceLen = __min(nUsbOnceLen, nAllLen-i);

		memset(arrWriteBuf, 0xFF, sizeof(arrWriteBuf));
		memcpy(arrWriteBuf, &arrRomBuf[DEF_APP_CODE_START_ADDR+i], nOnceLen);
		
		//nRet = CommControl::GetInstance()->Stm32WriteFlash(nStartAddr+i, &arrRomBuf[DEF_APP_CODE_START_ADDR+i], nOnceLen);
		nRet = CommControl::GetInstance()->Stm32WriteFlash(nStartAddr+i, arrWriteBuf, nUsbOnceLen);
		if(nRet != 0)
			break;
		
		m_fUpgradeProgress  = nOnceLen + i;
		m_fUpgradeProgress /= 1.0*nAllLen;

		while(nLastPrintCnt--)
			putchar('\b');
		
		sprintf(strMsg, "write flash: %.2f%s", m_fUpgradeProgress*100, "%");
		printf("%s", strMsg);
        fflush (stdout);
		nLastPrintCnt = strlen(strMsg);
	}
	printf("\n");
	nLastPrintCnt = 0;

	for(i=0; i<nAllLen; i += nUsbOnceLen)
	{
		nOnceLen = __min(nUsbOnceLen, nAllLen-i);
		
		nRet = CommControl::GetInstance()->Stm32ReadFlash(nStartAddr+i, arrReadBuf, nOnceLen);
		if(nRet == 0)
			nRet = memcmp(arrReadBuf, &arrRomBuf[DEF_APP_CODE_START_ADDR+i], nOnceLen);
		
		if(nRet != 0)
			break;
		
		m_fUpgradeProgress  = nOnceLen + i;
		m_fUpgradeProgress /= 1.0*nAllLen;

		while(nLastPrintCnt--)
			putchar('\b');
		
		sprintf(strMsg, "check flash: %.2f%s", m_fUpgradeProgress*100, "%");
		printf("%s", strMsg);
        fflush (stdout);
		nLastPrintCnt = strlen(strMsg);
	}
	printf("\n");

	if(nRet !=0 )
		goto END;

	nRet = ClearUpdateFlag();
	if(nRet !=0 )
		goto END;

	nRet = WriteUpdateFlag();
	if(nRet !=0 )
		goto END;

	nRet = CheckUpdateFlag();
	if(nRet !=0 )
		goto END;

	m_fUpgradeProgress = 1;
	SoftReset();

	nRet = GetAppFirmware(&nVersion);

	if(nRet==0)
	{
		printf("app.%03lu\n", nVersion);
		CommControl::GetInstance()->LeaveUsb();
	}

END:
	delete [] arrRomBuf;
	
	return nRet;
}

u8 Stm32::ReadFlash(u32 addr, u8 buf[], u32 len)
{
	u8 nRet = 0;
	u32 i, nOnceLen;
	const u32 nUsbOnceLen = 256;

	for(i=0, i=0; i<len; i += nUsbOnceLen)
	{
		nOnceLen = __min(nUsbOnceLen, len-i);
		nRet = CommControl::GetInstance()->Stm32ReadFlash(addr+i, &buf[i], nOnceLen);
		if(nRet != 0)
			break;
	}

	return nRet;
}

u8 Stm32::WriteFlash(u32 addr, const u8 buf[], u32 len)
{
	u8 nRet = 0;
	u32 i, nOnceLen;
	const u32 nUsbOnceLen = 256;

	for(i=0, i=0; i<len; i += nUsbOnceLen)
	{
		nOnceLen = __min(nUsbOnceLen, len-i);
		nRet = CommControl::GetInstance()->Stm32WriteFlash(addr+i, &buf[i], nOnceLen);
		if(nRet != 0)
			break;
	}

	return nRet;
}

u8 Stm32::ServerConfig_ReadFlash(u32 addr, u8 buf[], u32 len)
{
	return ReadFlash(addr, buf, len) ?SERVER_CONFIG_ERR_READ_FLASH :0;
}

u8 Stm32::ServerConfig_WriteFlash(u32 addr, const u8 buf[], u32 len)
{
	u8 nRet = 0;
	u8 readBuf[DEF_STM32HL_FLASH_PAGE_SIZE];

	u32 i;
	u32 nPageAddr, nStart, nEnd;
	u32 nPageStart = addr / DEF_STM32HL_FLASH_PAGE_SIZE;
	u32 nPageEnd = (addr+len-1) / DEF_STM32HL_FLASH_PAGE_SIZE;

	for(i=nPageStart; i<=nPageEnd; i++)
	{
		nPageAddr = i * DEF_STM32HL_FLASH_PAGE_SIZE;

		if((nRet = ReadFlash(nPageAddr, readBuf, DEF_STM32HL_FLASH_PAGE_SIZE)) != 0)
			break;

		nStart = __max(nPageAddr, addr);
		nEnd = __min(nPageAddr+DEF_STM32HL_FLASH_PAGE_SIZE-1, addr+len-1);

		memcpy(&readBuf[nStart-nPageAddr], &buf[nStart-addr], nEnd-nStart+1);

		if((nRet = CommControl::GetInstance()->Stm32EraseFlash(nPageAddr, DEF_STM32HL_FLASH_PAGE_SIZE)) != 0)
			break;

		if((nRet = WriteFlash(nPageAddr, readBuf, DEF_STM32HL_FLASH_PAGE_SIZE)) != 0)
			break;
	}

	return (nRet!=0) ?SERVER_CONFIG_ERR_WRITE_FLASH :nRet;
}
