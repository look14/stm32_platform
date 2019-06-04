#pragma warning(disable: 4786)

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

#include "tk_tcl.h"
#include "server_comm_control.h"
#include "server_comm_protocol.h"
#include "server_config.h"

#include "stm32.h"
#include "mpu6050.h"

u32 get_number(const string str)
{
	u32 number = 0;

	string strTemp = str;
	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);

	if(strTemp.size()>2 && (int)strTemp.find("0x") >= 0) {
		sscanf(strTemp.c_str(), "0x%x", &number);
	}
	else {
		sscanf(strTemp.c_str(), "%u", &number);
	}

	return number;
}

int get_cmd_list(vector<string> &vecCmd, char *strBuf)
{
	string strTemp;
	vecCmd.clear();

	for(char* p=strtok(strBuf, " "); p; p=strtok(NULL, " ")) {  
		strTemp = p;

		strTemp.erase(0, strTemp.find_first_not_of(" \r\n")); 
		strTemp.erase(strTemp.find_last_not_of(" \r\n") + 1);
		//	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), tolower);

		vecCmd.push_back(strTemp);
	}

	return vecCmd.size();
}

extern "C" __declspec(dllexport) int Stmusb_Init(Tcl_Interp *interp)
{
	Tcl_CreateCommand(interp, "get_pc_version",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "get_usb_vendor",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "get_usb_product",		(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "open_usb",				(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "close_usb",				(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "enter_usb",				(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "leave_usb",				(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "set_usb_vid",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "set_usb_pid",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "ping",					(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "get_firmware",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "soft_reset",				(Tcl_CmdProc*)TclCmdProc, NULL, NULL);

	Tcl_CreateCommand(interp, "set_gpio_out",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "set_gpio_in",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "set_gpio_high",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "set_gpio_low",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "read_gpio",				(Tcl_CmdProc*)TclCmdProc, NULL, NULL);

	Tcl_CreateCommand(interp, "config_check",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "config_clear",			(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "config_get_content",		(Tcl_CmdProc*)TclCmdProc, NULL, NULL);
	Tcl_CreateCommand(interp, "config_update_content",	(Tcl_CmdProc*)TclCmdProc, NULL, NULL);

	ServerConfig_Init(DEF_SYSTEM_SETTINGS_ADDR, Stm32::ServerConfig_ReadFlash, Stm32::ServerConfig_WriteFlash);
	
    return TCL_OK;
}

int TclCmdProc(ClientData clientData, Tcl_Interp *interp, int argc, char * CONST * argv)
{
	u8 nPos, nRet;
	int i;

	Stm32 stm32;
	Mpu6050 mpu6050;

	u8 addr;
	u16 len;
	u8 buf8[256];
	u16 buf16[256];
	u32 tmp32;

	u16 nCmdLen;
	char strBuf[2048];

	string strCmd, strTmp;
	vector<string> vecCmd;

	for(nPos=0; nPos<argc; nPos++) {
		char* split = " ";
		char* token = strtok(argv[nPos], split);

		while( token != NULL ) { 
			vecCmd.push_back(token); 
			token = strtok(NULL, split);
		}
	}

	nPos = 0;
	strCmd = vecCmd[nPos++];

	transform(strCmd.begin(), strCmd.end(), strCmd.begin(), ::tolower);
	
	if(strCmd=="get_pc_version") {
		const char* strVer = "v1.2";
		Tcl_SetObjResult(interp, Tcl_NewStringObj(strVer, strlen(strVer)));
	}

	else if(strCmd=="get_usb_vendor") {
		char* str = CommControl::GetInstance()->GetUsbVendor();
		Tcl_SetObjResult(interp, Tcl_NewStringObj(str, strlen(str)));
	}

	else if(strCmd=="get_usb_product") {
		char* str = CommControl::GetInstance()->GetUsbProduct();
		Tcl_SetObjResult(interp, Tcl_NewStringObj(str, strlen(str)));
	}

	else if(strCmd=="open_usb") {
		nRet = CommControl::GetInstance()->ReopenUsb();
		Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
	}

	else if(strCmd=="close_usb") {
		CommControl::GetInstance()->CloseUsb();
	}

	else if(strCmd=="enter_usb") {
		nRet = CommControl::GetInstance()->EnterUsb();
		Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
	}

	else if(strCmd=="leave_usb") {
		nRet = CommControl::GetInstance()->LeaveUsb();
		Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
	}

	else if(strCmd=="set_usb_vid") {
		CommControl::GetInstance()->m_nUsbVendorId = get_number(vecCmd[nPos++]);
	}

	else if(strCmd=="set_usb_pid") {
		CommControl::GetInstance()->m_nUsbProductId = get_number(vecCmd[nPos++]);
	}

	else if(strCmd=="ping") {
		nRet = stm32.Ping();
		Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
	}

	else if(strCmd=="get_firmware") {
		char strFirmware[32] = "";
		nRet = stm32.GetFirmware(strFirmware);
		Tcl_SetObjResult(interp, Tcl_NewStringObj(strFirmware, strlen(strFirmware)));
	}

	else if(strCmd=="soft_reset") {
		nRet = stm32.SoftReset();
		Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
	}

	else if(strCmd=="set_gpio_out") {
		nCmdLen = vecCmd.size() - nPos;
		
		if(nCmdLen >= 2) {
			u32 gpio_type	= get_number(vecCmd[nPos++]);
			u16 gpio_pin	= get_number(vecCmd[nPos++]);

			nRet = CommControl::GetInstance()->SetGpioOut(gpio_type, gpio_pin);
			Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
		}
	}

	else if(strCmd=="set_gpio_in") {
		nCmdLen = vecCmd.size() - nPos;
		
		if(nCmdLen >= 2) {
			u32 gpio_type	= get_number(vecCmd[nPos++]);
			u16 gpio_pin	= get_number(vecCmd[nPos++]);
			
			nRet = CommControl::GetInstance()->SetGpioIn(gpio_type, gpio_pin);
			Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
		}
	}

	else if(strCmd=="set_gpio_high") {
		nCmdLen = vecCmd.size() - nPos;
		
		if(nCmdLen >= 2) {
			u32 gpio_type	= get_number(vecCmd[nPos++]);
			u16 gpio_pin	= get_number(vecCmd[nPos++]);
			
			nRet = CommControl::GetInstance()->SetGpioHigh(gpio_type, gpio_pin);
			Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
		}
	}

	else if(strCmd=="set_gpio_low") {
		nCmdLen = vecCmd.size() - nPos;
		
		if(nCmdLen >= 2) {
			u32 gpio_type	= get_number(vecCmd[nPos++]);
			u16 gpio_pin	= get_number(vecCmd[nPos++]);
			
			nRet = CommControl::GetInstance()->SetGpioLow(gpio_type, gpio_pin);
			Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
		}
	}

	else if(strCmd=="read_gpio") {
		nCmdLen = vecCmd.size() - nPos;
		
		if(nCmdLen >= 2) {
			u32 gpio_type	= get_number(vecCmd[nPos++]);
			u16 gpio_pin	= get_number(vecCmd[nPos++]);
			u8 gpio_status;
			
			nRet = CommControl::GetInstance()->ReadGpio(gpio_type, gpio_pin, gpio_status);
			if(nRet==0) {
				Tcl_SetObjResult(interp, Tcl_NewIntObj(gpio_status));
			}
		}
	}

	else if(strCmd=="config_check") {
		nRet = ServerConfig_CheckAll();
		Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
	}

	else if(strCmd=="config_clear") {
		nRet = ServerConfig_ClearAll();
		Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
	}

	else if(strCmd=="config_get_content") {
		nCmdLen = vecCmd.size() - nPos;
		
		if(nCmdLen >= 1) 
		{
			u8* pbuf = NULL;
			u32 len32;
			u32 nId = get_number(vecCmd[nPos++]);
			nRet = ServerConfig_GetContentById(nId, NULL, &len32);

			if(nRet==0) {
				pbuf = new u8[len32];
				nRet = ServerConfig_GetContentById(nId, pbuf, &len32);
			}

			if(nRet==0) {
				Tcl_Obj* *objv = new Tcl_Obj*[len32];
				
				for(i=0; i<len32; i++) {
					sprintf(strBuf, "0x%02X", pbuf[i]);
					objv[i] = Tcl_NewStringObj(strBuf, strlen(strBuf));
				}
				
				Tcl_SetObjResult(interp, Tcl_NewListObj(len32, objv));
				
				delete [] objv;
			}

			if(pbuf)
				delete [] pbuf;
		}
	}

	else if(strCmd=="config_update_content") {
		nCmdLen = vecCmd.size() - nPos;
		
		if(nCmdLen >= 2) {
			u32 nId = get_number(vecCmd[nPos++]);
			u32 len32 = nCmdLen - 1;
			u8* pbuf = new u8[len32];
			
			for(i=0; i<len32; i++) {
				pbuf[i] = get_number(vecCmd[nPos++]);
			}

			nRet = ServerConfig_UpdateContentById(nId, pbuf, len32);
			Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));

			delete [] pbuf;
		}
	}

	else if(strCmd=="mpu6050_read_reg") 
	{
		nCmdLen = vecCmd.size() - nPos;

		if(nCmdLen >= 1) {
			addr = get_number(vecCmd[nPos++]);
			len  = (nCmdLen >= 2) ?get_number(vecCmd[nPos++]) :1;

			nRet = mpu6050.ReadNReg(addr, buf8, len);

			if(nRet==0) {
				Tcl_Obj* *objv = new Tcl_Obj*[len];

				for(i=0; i<len; i++) {
					sprintf(strBuf, "0x%02X", buf8[i]);
					objv[i] = Tcl_NewStringObj(strBuf, strlen(strBuf));
					//objv[i] = Tcl_NewIntObj(buf8[i]);
				}

				Tcl_SetObjResult(interp, Tcl_NewListObj(len, objv));

				delete [] objv;
			} 
		}
	}

	else if(strCmd=="mpu6050_write_reg")
	{
		nCmdLen = vecCmd.size() - nPos;

		if(nCmdLen >= 1) {
			addr = get_number(vecCmd[nPos++]);
			len  = nCmdLen - 1;

			for(i=0; i<len; i++) {
				buf8[i] = get_number(vecCmd[nPos++]);
			}

			nRet = mpu6050.WriteNReg(addr, buf8, len);

			Tcl_SetObjResult(interp, Tcl_NewIntObj(nRet));
		}
	}

	//Tcl_SetObjResult(interp, Tcl_NewIntObj(iResult));

	return TCL_OK;
}
