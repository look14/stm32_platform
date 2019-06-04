#include <string.h>
#include "task.h"
#include "system.h"
#include "usb_lib.h"
#include "usb_pwr.h"

#include "common.h"
#include "system.h"
#include "boot_loader.h"

#include "server_comm_protocol.h"
#include "server_comm_control.h"
#include "time_server.h"

u8 g_bOnlyUsbThreadRun = 0;
u32 g_nUsbRecvTimeout   = 1;

void task_init(void)
{
	/* system init */
	common_init();
	system_config();

	/* controller init */
	Comm_Init(&g_usbComm, 0);
	CommControl_Init();

	time_server_init();
}

void task_run(void)
{
	g_bOnlyUsbThreadRun = 0;

	while(1)
	{
		usb_thread_proc();

		if(0==g_bOnlyUsbThreadRun) {
			test_thread_proc();
		}
	}
}

u8 usb_thread_proc(void)
{
	static u8 sendBuf[COMM_MAX_DATA_SIZE];
	static u8 recvBuf[COMM_MAX_DATA_SIZE];
	static u16 nSendLen, nRecvLen;
	static u8 isInit = 0;

	u8 nCmd;

	if(isInit==0) {
		isInit = 1;

		nSendLen = 0;
		nRecvLen = 0;
		memset(sendBuf, 0, sizeof(sendBuf));
		memset(recvBuf, 0, sizeof(recvBuf));

		g_pCommCtrlRecvBuf = recvBuf;
		g_pCommCtrlRecvLen = &nRecvLen;
	}

	//if(0==CommController_RecvBuf(recvBuf, &nRecvLen, g_nUsbRecvTimeout))//0xFFFFFFFF))
	if(g_nCommCtrlRecvDoneFlag)
	{
		g_nCommCtrlRecvDoneFlag = 0;
		nCmd = recvBuf[0];

		switch(nCmd)
		{
		case COMM_CONTROL_PING:         CommControl_Ping(recvBuf, nRecvLen, sendBuf, &nSendLen);            break;
		case COMM_CONTROL_FIRMWARE:     CommControl_GetFirmware(recvBuf, nRecvLen, sendBuf, &nSendLen);     break;
		case COMM_CONTROL_SOFT_RESET:   CommControl_SoftReset(recvBuf, nRecvLen, sendBuf, &nSendLen);       break;
		case COMM_CONTROL_ENTER_USB:    CommControl_JustResponseOk(sendBuf, &nSendLen);                     break;
		case COMM_CONTROL_LEAVE_USB:    CommControl_JustResponseOk(sendBuf, &nSendLen);                     break;
		case COMM_CONTROL_STM32:        CommControl_Stm32Flash(recvBuf, nRecvLen, sendBuf, &nSendLen);      break;
		}

		CommControl_SendBuf(sendBuf, nSendLen, 1000);

		if(g_nStm32SoftResetFlag && nCmd==COMM_CONTROL_SOFT_RESET) {
			g_nStm32SoftResetFlag = 0;

			system_delay_ms(200);
			PowerOff();
			system_delay_ms(500);
			Bootloader_closeBsp();
			SYSTEM_Soft_Reset();
		}

		if(nCmd==COMM_CONTROL_ENTER_USB || nCmd==COMM_CONTROL_LEAVE_USB) {
			g_bOnlyUsbThreadRun = nCmd==COMM_CONTROL_ENTER_USB ? 1 : 0;
			g_nUsbRecvTimeout   = nCmd==COMM_CONTROL_ENTER_USB ? 0xFFFFFFFF : 10;
		}
	}

	return 0;
}

u8 test_thread_proc(void)
{
	return 0;
}
