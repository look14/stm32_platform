// test.cpp : 定义控制台应用程序的入口点。
//

#pragma warning(disable: 4786)

#include "stdafx.h"

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
#include "mpu6050.h"

#include "mpu6050_test.h"

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
		//	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), tolower);

		vecCmd.push_back(strTemp);
	}

	return vecCmd.size();
}

int _tmain(int argc, _TCHAR* argv[])
{
	//mpu6050_test();

	u8 nRet, nPos;

	Stm32 stm32;
	Mpu6050 mpu6050;

	u8 isDirectCmd;
	char strBuf[2048];
	string strCmd, strTmp;
	vector<string> vecCmd;

	/*double fTemp, fGravity=0;
	double fAccelX, fAccelY, fAccelZ;
	double fGyroX, fGyroY, fGyroZ;
	double angleXZ, angleYZ;
	int isInit = 0;

	double fAccelXOffset=0, fAccelYOffset=0, fAccelZOffset=0;
	double fGyroXOffset=0, fGyroYOffset=0, fGyroZOffset=0;
	double fTempOffset=0; 

	double fUsStart, fUsInterval;

	if(TRUE==mpu6050.Config())
	{
		Sleep(500);
		u32 count = 0;
		while(true)
		{
			mpu6050.GetCurrentData(fAccelX, fAccelY, fAccelZ, fGyroX, fGyroY, fGyroZ, fTemp);

			if(isInit < 10)
			{
				isInit++;

				fGravity += sqrt( fAccelX*fAccelX + fAccelY*fAccelY + fAccelZ*fAccelZ );

				if(fAccelX > 0)	angleXZ = atan2(fAccelX, fAccelZ)*180.0/3.14159265;
				else			angleXZ = atan2(fAccelX, fAccelZ)*180.0/3.14159265;

				if(fAccelY > 0)	angleYZ = atan2(fAccelY, fAccelZ)*180.0/3.14159265;
				else			angleYZ = atan2(fAccelY, fAccelZ)*180.0/3.14159265;

				fGyroXOffset  -= fGyroX ;
				fGyroYOffset  -= fGyroY ;
				fGyroZOffset  -= fGyroZ ;

				if(isInit > 1)
				{
					fGravity /= 2.0;
					fGyroXOffset  /= 2.0 ;
					fGyroYOffset  /= 2.0 ;
					fGyroZOffset  /= 2.0 ;
				}

				angleXZ = 0;
				angleYZ = 0;

				fUsStart = mpu6050.GetUsTimeCounter();
			}
			else
			{
				fAccelX += fAccelXOffset;
				fAccelY += fAccelYOffset;
				fAccelZ += fAccelZOffset;
				fGyroX  += fGyroXOffset;
				fGyroY  += fGyroYOffset;
				fGyroZ  += fGyroZOffset;
				fTemp   += fTempOffset;

				fUsInterval = mpu6050.GetUsTimeCounter() - fUsStart;
				fUsStart = mpu6050.GetUsTimeCounter();

				angleXZ -= fGyroY * fUsInterval / 1000000.0;
				angleYZ += fGyroX * fUsInterval / 1000000.0;

				if(angleXZ < -180)		angleXZ = angleXZ + 360;
				else if(angleXZ > 180)	angleXZ = angleXZ - 360;

				if(angleYZ < -180)		angleYZ = angleYZ + 360;
				else if(angleYZ > 180)	angleYZ = angleYZ - 360;
			}

			if(count++ % 100 != 0)	continue;

			//	printf("temperature: %.2f,  gravity: %.2f\n", fTemp, fGravity);
			//	printf("%8.2f %8.2f %8.2f , %10.4f %10.4f %10.4f\n", fAccelX,fAccelY,fAccelZ,fGyroX,fGyroY,fGyroZ);
			printf("angleXZ: %.2f,  angleYZ: %.2f\n", angleXZ, angleYZ);

			//	Sleep(1);
		}

	}*/

//	SYSTEMTIME tm;

//	GetLocalTime(&tm);

//	CommControl::GetInstance()->RtcUpdateTime(&tm);
//	CommControl::GetInstance()->RtcGetTime(&tm);


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


	//nRet = stm32.UpgradeAppCode("E:\\SVN\\design\\cmt2600\\trunk\\software\\cmt2600_demo_v1\\src\\stm32_demo_v1\\MDK-ARM\\Obj\\stm32_demo.hex");
	//printf("UpgradeAppCode: %u\n", nRet);

	return 0;
}

