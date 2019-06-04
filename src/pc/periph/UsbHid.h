#ifndef __USBHID_H
#define __USBHID_H

#undef WINVER
#define WINVER 0x0500 

#include "typedefs.h"
#include <afxwin.h>
#include <afxdisp.h>
#include <dbt.h>
#include <setupapi.h>

extern "C" {
#include ".\\ddk\\hidsdi.h"
}

#pragma comment(lib, ".\\ddk\\hid")
#pragma comment(lib, "setupapi")

typedef u8 (*pUsbHid_RecvCallback)(void* pParam, const u8 recvBuf[], u16 recvLen);


class CUsbHid
{
public:
	CUsbHid();
	virtual ~CUsbHid();

	virtual BOOL Close(void);
	virtual BOOL Open (CWnd *pPortOwner, DWORD VID, DWORD PID);

	virtual void Lock();
	virtual void Unlock();

	virtual UINT Send(const UCHAR *pSendBytes,UINT nSendLen);
	virtual UINT Recv(UCHAR *pRecvBytes);
	void GetDevicePath(CString &str);
	void RegisterRecvCallback(pUsbHid_RecvCallback pCallback, void* pParam) { 
		m_pUsbHid_RecvCallback = pCallback; 
		m_pUsbHid_RecvParam = pParam;
	}

	BOOL Ready(void);							// USB是否已经打开

protected:
	BOOL CreateThreadAndEvent(void);			// 创建线程和事件
	static DWORD RecvThread(LPVOID lpArg);		// 接收线程 
	void RegisterHIDDevice(CWnd *pPortOwner, GUID HidGuid);
	void TrimStringProperty(char *buf, size_t len);

private:

	CWnd *     m_pOwner;

	BOOL       m_bInit;

	HIDP_CAPS  m_Capabilities;

	OVERLAPPED m_ovUSBRead;
	OVERLAPPED m_ovUSBWrite;

	HANDLE     m_hDevHandle;
	HANDLE     m_hUSBWrite;
	HANDLE     m_hUSBRead;
	HANDLE     m_hReceivedEvent;
	HANDLE     m_hRecvExitEvent; 
	HANDLE     m_hConfirmExitEvent; 

	UCHAR     *m_szUSBRecvBuf;
	
	UINT       m_nRecvLength;

	CString    m_strDevicePath;

	CRITICAL_SECTION		m_csUsbHid;
	pUsbHid_RecvCallback	m_pUsbHid_RecvCallback;
	void*					m_pUsbHid_RecvParam;

public:
	UINT       m_nUSBRecvBufSize;
	UINT       m_nUSBSendBufSize;
	UINT       m_nRecvTimeout;
	UINT       m_nSendTimeout;

	char       m_strVendor[256];
	char       m_strProduct[256];
	char       m_strSerialNumber[256];
};


#endif