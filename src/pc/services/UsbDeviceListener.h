#ifndef __USB_DEVICE_LISTENER_H
#define __USB_DEVICE_LISTENER_H

#include "typedefs.h"
#include "UsbHid.h"

typedef UINT(*pUsbDeviceListenerCallback)(LPVOID pParam);

struct UsbListenerData{
	UsbListenerData()
	{
		Clear();
	}
	~UsbListenerData()
	{
	}
	void Clear()
	{
		dwThreadID		= 0;
		dwWndThreadID	= 0;
		hThread			= NULL;
		hWndThread		= NULL;
		hCloseEvent		= NULL;
		hWaitCloseEvent	= NULL;
		hUsbOpenEvent	= NULL;
		hUsbCloseEvent	= NULL;
		hWnd            = NULL;
		isClose			= TRUE;
	}
	
	BOOL	isClose;
	HWND    hWnd;
	DWORD	dwThreadID;
	DWORD	dwWndThreadID;
	HANDLE	hThread;
	HANDLE	hWndThread;
	HANDLE	hCloseEvent;
	HANDLE	hWaitCloseEvent;
	HANDLE	hUsbOpenEvent;
	HANDLE	hUsbCloseEvent;
};

void SetUsbListenerOpenCallback(pUsbDeviceListenerCallback pCallback, LPVOID pParam);
void SetUsbListenerCloseCallback(pUsbDeviceListenerCallback pCallback, LPVOID pParam);

void UsbListenerStart();
void UsbListenerStop();
UINT UsbListener(LPVOID lpVoid);
UINT UsbListenerWnd(LPVOID lpVoid);

INT_PTR WINAPI UsbListenerWinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND CreateUsbListenerWndClass(const char* WND_CLASS_NAME);

extern UsbListenerData usbListenerData;


#endif
