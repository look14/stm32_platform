#include "server_comm_control.h"
#include "common.h"
#include "system.h"
#include "time_server.h"
#include "usb_lib.h"

#include "rtc.h"
#include "mpu6050.h"

#include <string.h>

u8 g_nCommCtrlRecvDoneFlag;
u16 *g_pCommCtrlRecvLen;     
u8 *g_pCommCtrlRecvBuf;

u16 g_nCommCtrlSendLen;
u16 g_nCommCtrlRecvLen;
u8  g_strCommCtrlSendBuf[COMM_MAX_DATA_SIZE];
u8  g_stCommControlRecvBuf[COMM_MAX_DATA_SIZE];
u8  g_strCommCtrlTmpBuf8[COMM_MAX_DATA_SIZE];
u16 g_strCommCtrlTmpBuf16[COMM_MAX_DATA_SIZE/2];

Comm_Type g_usbComm;

const u8 g_strFirmware[] = "app.001";

void CommControl_Init(void)
{
	g_pCommCtrlRecvBuf      = NULL;
	g_pCommCtrlRecvLen      = NULL;  
	g_nCommCtrlRecvDoneFlag = 0; 
}

u8 CommControl_PhySend(void* pParam, const u8 buf[], u16 len, u32 timeout)
{
	u16 i;
	u16 nMaxLen = 60;
	u16 nCurLen;

	SetUserTimeout(timeout);

	for(i = 0; i < len; i += nMaxLen)
	{
		nCurLen = len - i;
		nCurLen = nMaxLen<nCurLen ?nMaxLen :nCurLen;

		//g_usbSendBuf[0] = 0x3F;
		memset(&g_usbSendBuf[0], 0, USB_BUF_SIZE);
		memcpy(&g_usbSendBuf[0], &buf[i], nCurLen);

		UserToPMABufferCopy(g_usbSendBuf, ENDP2_TXADDR, USB_BUF_SIZE);
        SetEPTxCount(ENDP2, USB_BUF_SIZE);
        SetEPTxValid(ENDP2);

		while(GetEPTxStatus(ENDP2)!=EP_TX_NAK && IsUserTimeout()==0);

		if(IsUserTimeout())
		{
			return COMM_ERR_TIMEOUT_SEND;
		}

	//	SYSTEM_TimerDelay_Ms(10);
	}

	return COMM_ERR_OK;
}

u8 CommControl_SendRetry(u32 timeout)
{
	return Comm_SendRetry(&g_usbComm, timeout, CommControl_PhySend, NULL);
}

u8 CommControl_SendBuf(const u8 buf[], u16 len, u32 timeout)
{
	return Comm_SendBytes(&g_usbComm, buf, len, timeout, CommControl_PhySend, NULL);
}

u8 CommControl_RecvBuf(u8 buf[], u16 *pLen, u32 timeout)
{  
	g_nCommCtrlRecvDoneFlag = 0;
	g_pCommCtrlRecvBuf      = buf;
	g_pCommCtrlRecvLen      = pLen;

	CommControl_RecvThread();

	SetUserTimeout(timeout);
	while(g_nCommCtrlRecvDoneFlag==0 && IsUserTimeout()==0);

	g_pCommCtrlRecvBuf = NULL;
	g_pCommCtrlRecvLen = NULL;

	return g_nCommCtrlRecvDoneFlag ?COMM_ERR_OK :COMM_ERR_TIMEOUT_RECV;
}

u8 CommControl_RecvThread(void)
{ 
#if 1
    u16 i;
	u8  nRet; 
    u16 recvLen = 0;
    
    if(g_bUsbRecvReady) {
		g_bUsbRecvReady = 0;

		PMAToUserBufferCopy(g_usbRecvBuf, ENDP1_RXADDR, USB_BUF_SIZE);
      	//SetEPRxStatus(ENDP1, EP_RX_VALID);
        SetEPRxValid(ENDP1);
        
        recvLen = 60;
	}
    
    if(recvLen > 0 && g_pCommCtrlRecvBuf != NULL && g_pCommCtrlRecvLen != NULL)
	{
		for(i=0; i<recvLen; i++)
		{
			if(g_pCommCtrlRecvBuf == NULL || g_pCommCtrlRecvLen == NULL)
				break;

			nRet = Comm_RecvBytes(&g_usbComm, g_usbRecvBuf[i], g_pCommCtrlRecvBuf, g_pCommCtrlRecvLen);

			if(COMM_ERR_OK == nRet) {
				g_nCommCtrlRecvDoneFlag = 1;
				break;
			}
			else if(COMM_ERR_RECV_REPEAT==nRet) { 
					CommControl_SendRetry(1000);
					break;
			}
		}

		recvLen = 0;
	}
    
#else
	u16 i;
	u8  nRet; 

	#define MAX_LEN	(60*5)

	static u8 buf[MAX_LEN];
	static u16 recvLen = 0;

	if(g_bUsbRecvReady)
	{
		g_bUsbRecvReady = 0;

		PMAToUserBufferCopy(g_usbRecvBuf, ENDP1_RXADDR, USB_BUF_SIZE);
      	SetEPRxStatus(ENDP1, EP_RX_VALID);
		
		if(MAX_LEN-recvLen >= 60) {
			memcpy(&buf[recvLen], &g_usbRecvBuf[0], 60);
			recvLen += 60;
		}
		else {
			recvLen = 0;
		}
	}

	if(recvLen > 0 && g_pCommCtrlRecvBuf != NULL && g_pCommCtrlRecvLen != NULL)
	{
		//for(i=1; i<=60; i++)
		for(i=0; i<recvLen; i++)
		{
			if(g_pCommCtrlRecvBuf == NULL || g_pCommCtrlRecvLen == NULL)
				break;

			nRet = Comm_RecvBytes(&g_usbComm, buf[i], g_pCommCtrlRecvBuf, g_pCommCtrlRecvLen);

			if(COMM_ERR_OK == nRet) {
				g_nCommCtrlRecvDoneFlag = 1;
				break;
			}
			else if(COMM_ERR_RECV_REPEAT==nRet) { 
				CommControl_SendRetry(1000);
				break;
			}
		}

		recvLen = 0;
	}
#endif

	return 0;
}

void CommControl_U16ToU8(const u16 buf16[], u8 buf8[], u16 len8)
{
	u16 i;
	for(i = 0; i < len8/2; i++)
	{
		buf8[2*i]   = (u8)(buf16[i]);
		buf8[2*i+1] = (u8)(buf16[i]>>8);
	}
}

void CommControl_U8ToU16(const u8 buf8[], u16 buf16[], u16 len8)
{
	u16 i;
	for(i = 0; i < len8/2; i++)
	{
		buf16[i]   = buf8[2*i+1];
		buf16[i] <<= 8;
		buf16[i]  |= buf8[2*i];
	}
}

u8 CommControl_IcBufDecode(const u8 recvBuf[], u16 recvLen, u8* nCmd, u8* nSubCmd, u8* addr, u16* len, const u8** pbuf)
{
	u8 index = 0;

	*nCmd = recvBuf[index];
	index += 1;

	*nSubCmd = recvBuf[index];
	index += 1;

	*addr = recvBuf[index];
	index += 1;

	*len = COMM_CONTROL_GETU16(&recvBuf[index]);
	index += 2;
	
	*pbuf = &recvBuf[index];

	if(index > recvLen)
		return 1;

	return 0;
}

u8 CommControl_Stm32FlashDecode(const u8 recvBuf[], u16 recvLen, u8* nCmd, u8* nSubCmd, u32* addr, u32* len, const u8** pbuf)
{
	u8 index = 0;

	*nCmd = recvBuf[index];
	index += 1;

	*nSubCmd = recvBuf[index];
	index += 1;

	*addr = COMM_CONTROL_GETU32(&recvBuf[index]);
	index += 4;

	*len = COMM_CONTROL_GETU32(&recvBuf[index]);
	index += 4;
	
	*pbuf = &recvBuf[index];

	if(index > recvLen)
		return 1;

	return 0;
}

u8 CommControl_JustResponseOk(u8 sendBuf[], u16 *pSendLen)
{
	sendBuf[0] = 0;
	*pSendLen  = 1;

	return 0;
}

u8 CommControl_Ping(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen)
{
	return CommControl_JustResponseOk(sendBuf, pSendLen);
}

u8 CommControl_SoftReset(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen)
{
	g_nStm32SoftResetFlag = 1;
	return CommControl_JustResponseOk(sendBuf, pSendLen);
}

u8 CommControl_GetFirmware(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen)
{
	u16 len = strlen((char*)g_strFirmware) + 1;

	sendBuf[0] = 0;
	memcpy(&sendBuf[1], g_strFirmware, len);
	*pSendLen  = len + 1;

	return 0;
}

u8 CommControl_Stm32Flash(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen) 
{
	u8 nCmd, nSubCmd;
	u32 addr;
	u32 len;
	const u8* pInBuf;
	
	u8 nRet = 0;
	u8* pResult = &sendBuf[0];
	u8* pOutBuf = &sendBuf[1];

	if(0 != CommControl_Stm32FlashDecode(recvBuf, recvLen, &nCmd, &nSubCmd, &addr, &len, &pInBuf))
	{
		*pResult  = COMM_CONTROL_ERR_DATA_INFO;
		*pSendLen = 1;
		return 1;
	}

	switch(nSubCmd)
	{
	case COMM_CONTROL_STM32_READ_FLASH:
	{
		nRet = flash_read(addr, pOutBuf, len);
		*pResult  = nRet==0 ?COMM_CONTROL_ERR_OK :COMM_CONTROL_ERR_READ_FLASH;
		*pSendLen = 1 + len;
	}
	break;

	case COMM_CONTROL_STM32_ERASE_FLASH:
	{
		nRet = flash_erase(addr, len);
		*pResult  = nRet==0 ?COMM_CONTROL_ERR_OK :COMM_CONTROL_ERR_ERASE_FLASH;
		*pSendLen = 1;
	}
	break;

	case COMM_CONTROL_STM32_WRITE_FLASH:
	{
		nRet = flash_write(addr, pInBuf, len);
		*pResult  = nRet==0 ?COMM_CONTROL_ERR_OK :COMM_CONTROL_ERR_WRITE_FLASH;
		*pSendLen = 1;
	}
	break;
	}

	return 0;
}

u8 CommControl_GpioOpera(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen)
{
    u8 nCmd;
    u8 nSubCmd;

	u8* pResult = &sendBuf[0];
	u8* pOutBuf = &sendBuf[1];
    u8 index = 0;
    
    u32 gpio_type;
    u16 gpio_pin;
	
	nCmd        = recvBuf[index++];
	nSubCmd     = recvBuf[index++];
    gpio_type   = COMM_CONTROL_GETU32(&recvBuf[index]);  index += 4;
    gpio_pin    = COMM_CONTROL_GETU16(&recvBuf[index]);  index += 2;
    
	if(COMM_CONTROL_GPIO_OUT==nSubCmd) {
        GPIO_Pin_Setting((GPIO_TypeDef*)gpio_type, gpio_pin, GPIO_Speed_50MHz, GPIO_Mode_Out_PP);
        CommControl_JustResponseOk(sendBuf, pSendLen);
	}

	else if(COMM_CONTROL_GPIO_IN==nSubCmd) {
        GPIO_Pin_Setting((GPIO_TypeDef*)gpio_type, gpio_pin, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING);
        CommControl_JustResponseOk(sendBuf, pSendLen);
	}

	else if(COMM_CONTROL_GPIO_HIGH==nSubCmd) {
        GPIO_SetBits((GPIO_TypeDef*)gpio_type, gpio_pin);
        CommControl_JustResponseOk(sendBuf, pSendLen);
	}

	else if(COMM_CONTROL_GPIO_LOW==nSubCmd) {
        GPIO_ResetBits((GPIO_TypeDef*)gpio_type, gpio_pin);
        CommControl_JustResponseOk(sendBuf, pSendLen);
	}
    
    else if(COMM_CONTROL_GPIO_READ==nSubCmd) {
        pOutBuf[0] = GPIO_ReadInputDataBit((GPIO_TypeDef*)gpio_type, gpio_pin);
        *pResult  = 0;
        *pSendLen = 2;
	}

	return 0;
}

u8 CommControl_Rtc(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen)
{
    u8 nCmd;
    u8 nSubCmd;

	u8* pResult = &sendBuf[0];
	u8* pOutBuf = &sendBuf[1];
    u8 index = 0;
    u8 index2 = 0;
	
	nCmd    = recvBuf[index++];
	nSubCmd = recvBuf[index++];
    
	if(COMM_CONTROL_RTC_UPDATE_TIME==nSubCmd) {
        g_rtc_date_time.year  = get_u16_from_buf(&recvBuf[index]);    index += 2;
		g_rtc_date_time.month = recvBuf[index++];
		g_rtc_date_time.day   = recvBuf[index++];
		g_rtc_date_time.hour  = recvBuf[index++];
		g_rtc_date_time.min   = recvBuf[index++];
		g_rtc_date_time.sec   = recvBuf[index++];
        g_rtc_date_time.week  = recvBuf[index++];
        rtc_set_time();
        
        *pResult = 0;
        *pSendLen = 1;
	}

	else if(COMM_CONTROL_RTC_GET_TIME==nSubCmd) {
        *pResult = rtc_get_time();
        
        set_u16_to_buf(&pOutBuf[index2], g_rtc_date_time.year);    index2 += 2;
		pOutBuf[index2++] = g_rtc_date_time.month;
		pOutBuf[index2++] = g_rtc_date_time.day;
		pOutBuf[index2++] = g_rtc_date_time.hour;
		pOutBuf[index2++] = g_rtc_date_time.min;
		pOutBuf[index2++] = g_rtc_date_time.sec;
        pOutBuf[index2++] = g_rtc_date_time.week;
        *pSendLen = index2 + 1;
	}

	return 0;
}

u8 CommControl_Mpu6050(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen)
{
    u8 nCmd, nSubCmd, addr;
	u16 len;
	const u8* pInBuf;
	
	u8 nRet = 0;
	u8* pResult = &sendBuf[0];
	u8* pOutBuf = &sendBuf[1];

	if(0 != CommControl_IcBufDecode(recvBuf, recvLen, &nCmd, &nSubCmd, &addr, &len, &pInBuf))
	{
		*pResult  = COMM_CONTROL_ERR_DATA_INFO;
		*pSendLen = 1;
		return 1;
	}
	
	switch(nSubCmd)
	{
	case COMM_CONTROL_IC_READ_REG:
	{
		nRet = Mpu6050_ReadNReg(addr, pOutBuf, len);
		*pResult  = nRet==0 ?COMM_CONTROL_ERR_OK :COMM_CONTROL_ERR_READ_REG;
		*pSendLen = 1 + len;
	}
	break;

	case COMM_CONTROL_IC_WRITE_REG:
	{
		nRet = Mpu6050_WriteNReg(addr, pInBuf, len);
		*pResult  = nRet==0 ?COMM_CONTROL_ERR_OK :COMM_CONTROL_ERR_WRITE_REG;
		*pSendLen = 1;
	}
	break;
	}

	return 0;
}

