//
//  main.cpp
//  stm32_pc
//
//  Created by look on 17/1/7.
//  Copyright (c) 2017年 look. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

#include "server_comm_control.h"
#include "server_comm_protocol.h"
#include "server_config.h"

#include "stm32.h"

int get_number(const string str)
{
    int number = 0;
    
    string strTemp = str;
    transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);
    
    if(strTemp.size()>2 && (int)strTemp.find("0x") >= 0) {
        sscanf(strTemp.c_str(), "0x%x", &number);
    }
    else {
        sscanf(strTemp.c_str(), "%d", &number);
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
    //	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);
        
        vecCmd.push_back(strTemp);
    }
    
    return (int)vecCmd.size();
}

int main(int argc, const char * argv[])
{
    u8 nRet, nPos;
    
    Stm32 stm32;
    
    u8 isDirectCmd;
    char strBuf[2048];
    string strCmd, strTmp;
    vector<string> vecCmd;
 
    ServerConfig_Init(DEF_SYSTEM_SETTINGS_ADDR, Stm32::ServerConfig_ReadFlash, Stm32::ServerConfig_WriteFlash);
    
    while(1)
    {
        vecCmd.clear();
        
        if(argc > 1) {
            isDirectCmd = 1;
            
            for(nPos=1; nPos<argc; nPos++) {
                vecCmd.push_back(argv[nPos]);
            }
        }
        else {
            isDirectCmd = 0;
            
            printf("stm32>");
            if(gets(strBuf))
                get_cmd_list(vecCmd, strBuf);
        }
        
        if(vecCmd.empty())
            continue;
        
        nPos = 0;
        strCmd = vecCmd[nPos++];
        
        transform(strCmd.begin(), strCmd.end(), strCmd.begin(), ::tolower);
        
        if(strCmd=="quit") {
            break;
        }
        
        else if(strCmd=="get_pc_version") {
            printf("%s: %s\n", strCmd.c_str(), "v1.1");
        }
        
        else if(strCmd=="get_usb_vendor") {
            printf("%s: %s\n", strCmd.c_str(), CommControl::GetInstance()->GetUsbVendor());
        }
        
        else if(strCmd=="get_usb_product") {
            printf("%s: %s\n", strCmd.c_str(), CommControl::GetInstance()->GetUsbProduct());
        }
        
        else if(strCmd=="open_usb") {
            printf("%s: %s\n", strCmd.c_str(), CommControl::GetInstance()->ReopenUsb()==TRUE ?"true" :"false");
        }
        
        else if(strCmd=="close_usb") {
            CommControl::GetInstance()->CloseUsb();
        }
        
        else if(strCmd=="enter_usb") {
            printf("%s: %s\n", strCmd.c_str(), CommControl::GetInstance()->EnterUsb()==0 ?"true" :"false");
        }
        
        else if(strCmd=="leave_usb") {
            printf("%s: %s\n", strCmd.c_str(), CommControl::GetInstance()->LeaveUsb()==0 ?"true" :"false");
        }
        
        else if(strCmd=="ping") {
            printf("%s: %s\n", strCmd.c_str(), stm32.Ping()==0 ?"true" :"false");
        }
        
        else if(strCmd=="get_firmware") {
            char strFirmware[32];
            printf("%s: %s\n", strCmd.c_str(), stm32.GetFirmware(strFirmware)==0 ?strFirmware :"false");
        }
        
        else if(strCmd=="soft_reset") {
            printf("%s: %s\n", strCmd.c_str(), stm32.SoftReset()==0 ?"true" :"false");
        }
        
        else if(strCmd=="upgrade") {
            if(nPos < vecCmd.size())
            {
                strTmp = vecCmd[nPos++];
                printf("%s: %s\n", strCmd.c_str(), stm32.UpgradeAppCode(strTmp.c_str())==0 ?"true" :"false");
            }
        }
        
        if(isDirectCmd) {
            break;
        }
    }
    
	
    //nRet = stm32.UpgradeAppCode("/Users/look/Files/Program/Embedded/stm32_platform_v1/bin/upgrade.hex");    
	//printf("%d\n", nRet);
    
    return 0;
}

