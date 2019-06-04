#include <string.h>
#include "task.h"
#include "system.h"
#include "usb_lib.h"
#include "usb_pwr.h"

#include "common.h"
#include "system.h"
#include "boot_loader.h"

#include "key.h"
#include "led.h"
#include "rtc.h"

#include "mpu6050.h"

#include "time_server.h"
#include "interrupt_proc_server.h"

#include "server_comm_protocol.h"
#include "server_comm_control.h"

u8 g_bOnlyUsbThreadRun = 0;
u32 g_nUsbRecvTimeout   = 1;

void task_init(void)
{
	/* system init */
	common_init();
	system_config();

	/* services init */
	time_server_init();
	int_proc_srv_init();

	/* periph init */
	key_init();
	led_init();
    rtc_init();

	/* drivers init */
	Mpu6050_Init();

	/* controller init */
	Comm_Init(&g_usbComm, 0);
	CommControl_Init();
}

void task_run(void)
{
	g_bOnlyUsbThreadRun = 0;

	while(1)
	{
		usb_thread_proc();

		if(0==g_bOnlyUsbThreadRun) {
            key_thread_proc();
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
		case COMM_CONTROL_GPIO_OPERA:   CommControl_GpioOpera(recvBuf, nRecvLen, sendBuf, &nSendLen);       break;
        case COMM_CONTROL_RTC:          CommControl_Rtc(recvBuf, nRecvLen, sendBuf, &nSendLen);             break;
        case COMM_CONTROL_MPU6050:      CommControl_Mpu6050(recvBuf, nRecvLen, sendBuf, &nSendLen);         break;
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

u8 key_thread_proc(void)
{
    if(g_key1_down_flag) {
        g_key1_down_flag = 0;
    }
    
    if(g_key2_down_flag) {
        g_key2_down_flag = 0;
    }
    
	return 0;
}

u8 test_thread_proc(void)
{
	return 0;
}
