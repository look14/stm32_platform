#include "stdafx.h"
#include "server_comm_control.h"

#include <string>
#include <ctime>
#include <queue>
using namespace std;

u16 USB_VID = 0x0483;
u16 USB_PID = 0x5750;

#define COMM_CONTROL_SNED_RESET()			m_nCommCtrlSendLen = 0;
#define COMM_CONTROL_SNED_ADD_U8(x)			m_strCommCtrlSendBuf[m_nCommCtrlSendLen] = x;						m_nCommCtrlSendLen += 1;
#define COMM_CONTROL_SNED_ADD_U16(x)		COMM_CONTROL_SETU16(&m_strCommCtrlSendBuf[m_nCommCtrlSendLen], x);	m_nCommCtrlSendLen += 2;
#define COMM_CONTROL_SNED_ADD_U32(x)		COMM_CONTROL_SETU32(&m_strCommCtrlSendBuf[m_nCommCtrlSendLen], x);	m_nCommCtrlSendLen += 4;
#define COMM_CONTROL_SNED_ADD_BUF(x, len)	memcpy(&m_strCommCtrlSendBuf[m_nCommCtrlSendLen], x, len);			m_nCommCtrlSendLen += len;

CommControl::CommControl()
{
	m_nUsbVendorId = 0x0483;
	m_nUsbProductId = 0x5750;

	m_nCommCtrlRecvDoneFlag = 0;
	m_pCommCtrlRecvLen      = 0;
	m_pCommCtrlRecvBuf      = 0;
	m_nCommCtrlSendLen      = 0;
	m_nCommCtrlRecvLen      = 0;

	memset(m_strCommCtrlSendBuf, 0, sizeof(m_strCommCtrlSendBuf));
	memset(m_strCommCtrlRecvBuf, 0, sizeof(m_strCommCtrlRecvBuf));
	memset(m_strCommCtrlTmpBuf8, 0, sizeof(m_strCommCtrlTmpBuf8));
	memset(m_strCommCtrlTmpBuf16, 0, sizeof(m_strCommCtrlTmpBuf16));

	Comm_Init(&m_usbComm, 1);
	Init();
}

CommControl::~CommControl()
{
	
}

CommControl* CommControl::GetInstance()
{
	static CommControl srcControl;
	return &srcControl;
}

BOOL CommControl::ReopenUsb()
{
	if(m_usbHid.Ready())
		m_usbHid.Close();

	return m_usbHid.Open(m_nUsbVendorId, m_nUsbProductId);
}

void CommControl::CloseUsb()
{
	m_usbHid.Close();
}

void CommControl::Init(void)
{
	m_pCommCtrlRecvBuf      = NULL;
	m_pCommCtrlRecvLen      = NULL;
	m_nCommCtrlRecvDoneFlag = 0;

	if(TRUE==ReopenUsb()){
		m_usbHid.RegisterRecvCallback(RecvCallback, this);
        m_usbHid.ResisterRemovalCallback(UsbRemovalCallback, this);
	}
	else {
		printf("open usb is failed!\n");
	}
}

u8 CommControl::PhySend(void* pParam, const u8 buf[], u16 len, u32 timeout)
{
	u16 i;
	u16 nCurLen;
	const u16 nMaxLen = 60;
    u8 sendBuf[nMaxLen];
    
    CommControl* pCtrl = (CommControl*)pParam;
    
    if(FALSE==pCtrl->m_usbHid.Ready()) {
        pCtrl->m_usbHid.Open(pCtrl->m_nUsbVendorId, pCtrl->m_nUsbProductId);
    }

	for(i = 0; i < len; i += nMaxLen)
	{
		nCurLen = len - i;
		nCurLen = __min(nMaxLen, nCurLen);

		memset(sendBuf, 0, sizeof(sendBuf));
		memcpy(sendBuf, &buf[i], nCurLen);

		if(pCtrl->m_usbHid.Send(sendBuf, nMaxLen, (int)timeout) < 0) {
			return COMM_ERR_SEND;
		}
	}

	return COMM_ERR_OK;
}

u8 CommControl::SendRetry(u32 timeout)
{
	return Comm_SendRetry(&m_usbComm, timeout, CommControl::PhySend, this);
}

u8 CommControl::SendBuf(const u8 buf[], u16 len, u32 timeout)
{
	return Comm_SendBytes(&m_usbComm, buf, len, timeout, CommControl::PhySend, this);
}

u8 CommControl::RecvBuf(u8 buf[], u16 *pLen, u32 timeout)
{  
	m_nCommCtrlRecvDoneFlag = 0;
	m_pCommCtrlRecvBuf      = buf;
	m_pCommCtrlRecvLen      = pLen;

	CommControl::RecvCallback(this, NULL, 0);
    
    int i, step = 1; // 1ms
    
    for(i=0; i<timeout; i+=step, usleep(step*1000))
    {
        if(m_nCommCtrlRecvDoneFlag)
            break;
    }

	m_pCommCtrlRecvBuf = NULL;
	m_pCommCtrlRecvLen = NULL;

	return m_nCommCtrlRecvDoneFlag ?COMM_ERR_OK :COMM_ERR_TIMEOUT_RECV;
}

u8 CommControl::Transfer(u8 sendBuf[], u16 sendLen, u8 recvBuf[], u16 *recvLen, u32 timeout)
{
	u8 nRet;
	u8 i, nRetryCnt = 1;

	//u32 clk_beg = clock();

	*recvLen = 0;

    nRet = CommControl::SendBuf(sendBuf, sendLen, timeout);
    if(nRet==COMM_ERR_OK)
		nRet = CommControl::RecvBuf(recvBuf, recvLen, timeout);

	if(nRet!=COMM_ERR_OK)
	{
		for(i=0; i<nRetryCnt; i++)
		{
			nRet = Comm_SendRetry(&m_usbComm, timeout, CommControl::PhySend, this);
			if(nRet==COMM_ERR_OK)
				nRet = CommControl::RecvBuf(recvBuf, recvLen, timeout);

			if(nRet==COMM_ERR_OK)
				break;
		}
	}

	//printf("%d ms\t", clock()-clk_beg);

	return nRet;
}

void CommControl::UsbRemovalCallback(void* pParam)
{
    CommControl* pCtrl = (CommControl*)pParam;
    
    if(pCtrl) {
        Comm_SendReset(&pCtrl->m_usbComm);
        Comm_RecvReset(&pCtrl->m_usbComm);
    }
}

u8 CommControl::RecvCallback(void* pParam, const u8 recvBuf[], u16 recvLen)
{
	u8 nRet;
	u16 needLen = 60;

	static queue<u8> queRecv;
    
    CommControl* pCtrl = (CommControl*)pParam;

	pCtrl->m_usbHid.Lock();

	if(recvLen >= needLen) 
	{
		for(int i=0; i<needLen; i++)
			queRecv.push(recvBuf[i]);
	}

	while(!queRecv.empty())
	{
		if(pCtrl->m_pCommCtrlRecvBuf == NULL || pCtrl->m_pCommCtrlRecvLen == NULL)
			break;

		nRet = Comm_RecvBytes(&pCtrl->m_usbComm, queRecv.front(), pCtrl->m_pCommCtrlRecvBuf, pCtrl->m_pCommCtrlRecvLen);
		queRecv.pop();

		if(COMM_ERR_OK == nRet)
		{
			pCtrl->m_nCommCtrlRecvDoneFlag = 1;
			break;
		}
	}

	pCtrl->m_usbHid.Unlock();

	return 0;
}

void CommControl::U16ToU8(const u16 buf16[], u8 buf8[], u16 len8)
{
	u16 i;
	for(i = 0; i < len8/2; i++)
	{
		buf8[2*i]   = (u8)(buf16[i]);
		buf8[2*i+1] = (u8)(buf16[i]>>8);
	}
}

void CommControl::U8ToU16(const u8 buf8[], u16 buf16[], u16 len8)
{
	u16 i;
	for(i = 0; i < len8/2; i++)
	{
		buf16[i]   = buf8[2*i+1];
		buf16[i] <<= 8;
		buf16[i]  |= buf8[2*i];
	}
}

u8 CommControl::SendCmd(u8 nCmd)
{
	u8 nRet;

	m_strCommCtrlSendBuf[0] = nCmd;
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, 1, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	return nRet==0? COMM_CONTROL_ERR_OK :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::IcReadBuf(u8 nCmd, u8 nSubCmd, u8 addr, u8 buf[], u16 len)
{
	u8 nRet;

	m_nCommCtrlSendLen = 0;

	m_strCommCtrlSendBuf[m_nCommCtrlSendLen] = nCmd;
	m_nCommCtrlSendLen += 1;

	m_strCommCtrlSendBuf[m_nCommCtrlSendLen] = nSubCmd;
	m_nCommCtrlSendLen += 1;

	m_strCommCtrlSendBuf[m_nCommCtrlSendLen] = addr;
	m_nCommCtrlSendLen += 1;

	COMM_CONTROL_SETU16(&m_strCommCtrlSendBuf[m_nCommCtrlSendLen], len);
	m_nCommCtrlSendLen += 2;

	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	
	if(nRet==0 && m_strCommCtrlRecvBuf[0]==COMM_CONTROL_ERR_OK)
	{
		memcpy(buf, &m_strCommCtrlRecvBuf[1], len);
	}
	
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::IcWriteBuf(u8 nCmd, u8 nSubCmd, u8 addr, const u8 buf[], u16 len)
{
	u8 nRet;

	m_nCommCtrlSendLen = 0;

	m_strCommCtrlSendBuf[m_nCommCtrlSendLen] = nCmd;
	m_nCommCtrlSendLen += 1;

	m_strCommCtrlSendBuf[m_nCommCtrlSendLen] = nSubCmd;
	m_nCommCtrlSendLen += 1;

	m_strCommCtrlSendBuf[m_nCommCtrlSendLen] = addr;
	m_nCommCtrlSendLen += 1;

	COMM_CONTROL_SETU16(&m_strCommCtrlSendBuf[m_nCommCtrlSendLen], len);
	m_nCommCtrlSendLen += 2;

	memcpy(&m_strCommCtrlSendBuf[m_nCommCtrlSendLen], buf, len);
	m_nCommCtrlSendLen += len;

	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);

	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::Ping()
{
	return CommControl::SendCmd(COMM_CONTROL_PING);
}

u8 CommControl::GetFirmware(u8 buf[], u16 *plen)
{
	u8 nRet;
	
	m_strCommCtrlSendBuf[0] = COMM_CONTROL_FIRMWARE;
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, 1, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	
	if(nRet==0 && m_strCommCtrlRecvBuf[0]==COMM_CONTROL_ERR_OK)
	{
		*plen = m_nCommCtrlRecvLen - 1;
		memcpy(buf, &m_strCommCtrlRecvBuf[1], *plen);
	}
	
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::SoftReset()
{
	return CommControl::SendCmd(COMM_CONTROL_SOFT_RESET);
}

u8 CommControl::EnterUsb()
{
	return CommControl::SendCmd(COMM_CONTROL_ENTER_USB);
}

u8 CommControl::LeaveUsb()
{
	return CommControl::SendCmd(COMM_CONTROL_LEAVE_USB);
}

u8 CommControl::Stm32ReadFlash(u32 addr, u8 buf[], u32 len)
{
	u8 nRet;

	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_STM32);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_STM32_READ_FLASH);
	COMM_CONTROL_SNED_ADD_U32(addr);
	COMM_CONTROL_SNED_ADD_U32(len);
	
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	
	if(nRet==0 && m_strCommCtrlRecvBuf[0]==COMM_CONTROL_ERR_OK)
	{
		memcpy(buf, &m_strCommCtrlRecvBuf[1], len);
	}
	
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::Stm32EraseFlash(u32 addr, u32 len)
{
	u8 nRet;

	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_STM32);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_STM32_ERASE_FLASH);
	COMM_CONTROL_SNED_ADD_U32(addr);
	COMM_CONTROL_SNED_ADD_U32(len);
	
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::Stm32WriteFlash(u32 addr, const u8 buf[], u32 len)
{
	u8 nRet;
	
	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_STM32);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_STM32_WRITE_FLASH);
	COMM_CONTROL_SNED_ADD_U32(addr);
	COMM_CONTROL_SNED_ADD_U32(len);
	COMM_CONTROL_SNED_ADD_BUF(buf, len);
	
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::SetGpioOut(u32 nGpioType, u16 nGpioPin)
{
	u8 nRet;
	
	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_OPERA);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_OUT);
	COMM_CONTROL_SNED_ADD_U32(nGpioType);
	COMM_CONTROL_SNED_ADD_U16(nGpioPin);

	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::SetGpioIn(u32 nGpioType, u16 nGpioPin)
{
	u8 nRet;
	
	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_OPERA);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_IN);
	COMM_CONTROL_SNED_ADD_U32(nGpioType);
	COMM_CONTROL_SNED_ADD_U16(nGpioPin);
	
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::SetGpioHigh(u32 nGpioType, u16 nGpioPin)
{
	u8 nRet;
	
	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_OPERA);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_HIGH);
	COMM_CONTROL_SNED_ADD_U32(nGpioType);
	COMM_CONTROL_SNED_ADD_U16(nGpioPin);
	
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::SetGpioLow(u32 nGpioType, u16 nGpioPin)
{
	u8 nRet;
	
	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_OPERA);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_LOW);
	COMM_CONTROL_SNED_ADD_U32(nGpioType);
	COMM_CONTROL_SNED_ADD_U16(nGpioPin);
	
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::ReadGpio(u32 nGpioType, u16 nGpioPin, u8 &nGpioStatus)
{
	u8 nRet;
	
	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_OPERA);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_GPIO_READ);
	COMM_CONTROL_SNED_ADD_U32(nGpioType);
	COMM_CONTROL_SNED_ADD_U16(nGpioPin);
	
	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);

	if(nRet==0 && m_strCommCtrlRecvBuf[0]==COMM_CONTROL_ERR_OK)
	{
		nGpioStatus = m_strCommCtrlRecvBuf[1];
	}

	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

#if 0
u8 CommControl::RtcUpdateTime(const SYSTEMTIME* ptm)
{
	u8 nRet;

	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_RTC);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_RTC_UPDATE_TIME);
	COMM_CONTROL_SNED_ADD_U16(ptm->wYear);
	COMM_CONTROL_SNED_ADD_U8(ptm->wMonth);
	COMM_CONTROL_SNED_ADD_U8(ptm->wDay);
	COMM_CONTROL_SNED_ADD_U8(ptm->wHour);
	COMM_CONTROL_SNED_ADD_U8(ptm->wMinute);
	COMM_CONTROL_SNED_ADD_U8(ptm->wSecond);
	COMM_CONTROL_SNED_ADD_U8(ptm->wDayOfWeek);

	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);
	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}

u8 CommControl::RtcGetTime(SYSTEMTIME* ptm)
{
	u8 nRet;

	COMM_CONTROL_SNED_RESET();
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_RTC);
	COMM_CONTROL_SNED_ADD_U8(COMM_CONTROL_RTC_GET_TIME);

	nRet = CommControl::Transfer(m_strCommCtrlSendBuf, m_nCommCtrlSendLen, m_strCommCtrlRecvBuf, &m_nCommCtrlRecvLen, 1000);

	if(nRet==0 && m_strCommCtrlRecvBuf[0]==COMM_CONTROL_ERR_OK)
	{
		u8 index = 1;

		ptm->wYear		= COMM_CONTROL_GETU16(&m_strCommCtrlRecvBuf[index]);	index += 2;
		ptm->wMonth		= m_strCommCtrlRecvBuf[index++];
		ptm->wDay		= m_strCommCtrlRecvBuf[index++];
		ptm->wHour		= m_strCommCtrlRecvBuf[index++];
		ptm->wMinute	= m_strCommCtrlRecvBuf[index++];
		ptm->wSecond	= m_strCommCtrlRecvBuf[index++];
		ptm->wDayOfWeek	= m_strCommCtrlRecvBuf[index++];
		ptm->wMilliseconds = 0;
	}

	return nRet==0? m_strCommCtrlRecvBuf[0] :COMM_CONTROL_ERR_COMM;
}
#endif

u8 CommControl::Mpu6050ReadRegs(u8 addr, u8 buf[], u16 len)
{
	return CommControl::IcReadBuf(COMM_CONTROL_MPU6050, COMM_CONTROL_IC_READ_REG, addr, buf, len);
}

u8 CommControl::Mpu6050WriteRegs(u8 addr, const u8 buf[], u16 len)
{
	return CommControl::IcWriteBuf(COMM_CONTROL_MPU6050, COMM_CONTROL_IC_WRITE_REG, addr, buf, len);
}
