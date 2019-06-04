#include "stdafx.h"
#include <stdio.h>
#include "UsbDeviceListener.h"
#include "server_comm_control.h"

UsbListenerData usbListenerData;

pUsbDeviceListenerCallback	g_usbDeviceListenerOpenCallback		= NULL;
pUsbDeviceListenerCallback	g_usbDeviceListenerCloseCallback	= NULL;
LPVOID	g_usbDeviceListenerOpenCallbackParam	= NULL;
LPVOID	g_usbDeviceListenerCloseCallbackParam	= NULL;

void SetUsbListenerOpenCallback(pUsbDeviceListenerCallback pCallback, LPVOID pParam)
{
	g_usbDeviceListenerOpenCallback = pCallback;
	g_usbDeviceListenerOpenCallbackParam = pParam;
}

void SetUsbListenerCloseCallback(pUsbDeviceListenerCallback pCallback, LPVOID pParam)
{
	g_usbDeviceListenerCloseCallback = pCallback;
	g_usbDeviceListenerCloseCallbackParam = pParam;
}

void UsbListenerStart()
{
	UsbListenerData &data = usbListenerData;
	UsbListenerStop();
	
	data.hCloseEvent		= CreateEvent(NULL, TRUE, FALSE, NULL);
	data.hWaitCloseEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);
	data.hUsbOpenEvent		= CreateEvent(NULL, TRUE, FALSE, NULL);
	data.hUsbCloseEvent		= CreateEvent(NULL, TRUE, FALSE, NULL);
	data.isClose			= FALSE;
	
	data.hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UsbListener, &data, 0, &data.dwThreadID);
	if(INVALID_HANDLE_VALUE==data.hThread)
	{
		printf("Create UsbListener thread failed!\n");
		data.hThread = NULL;
	}

	data.hWndThread = CreateThread(NULL, 0,	(LPTHREAD_START_ROUTINE)UsbListenerWnd, &data, 0, &data.dwWndThreadID);
	if(INVALID_HANDLE_VALUE==data.hWndThread)
	{
		printf("Create UsbListenerWnd thread failed!\n");
		data.hWndThread = NULL;
	}
	
	//	AfxBeginThread(FoundChipProc, (LPVOID)&fChipProcData);
}

void UsbListenerStop()
{
	UsbListenerData &data = usbListenerData;

	data.isClose = TRUE;
	if(data.hCloseEvent)
	{
		SetEvent(data.hCloseEvent);
		WaitForSingleObject(data.hWaitCloseEvent, 1000);
		Sleep(100);
	}
	if(data.hCloseEvent)		CloseHandle(data.hCloseEvent);
	if(data.hWaitCloseEvent)	CloseHandle(data.hWaitCloseEvent);
	if(data.hUsbOpenEvent)		CloseHandle(data.hUsbOpenEvent);
	if(data.hUsbCloseEvent)		CloseHandle(data.hUsbCloseEvent);
	if(data.hThread)			CloseHandle(data.hThread);
	if(data.hWndThread)			CloseHandle(data.hWndThread);
	
	data.Clear();
}

UINT UsbListener(LPVOID lpVoid)
{
	UsbListenerData &data = *(UsbListenerData*)lpVoid;

	const int EVENT_COUNT = 3;
	HANDLE hArrary[EVENT_COUNT];
	hArrary[0] = data.hCloseEvent;
	hArrary[1] = data.hUsbOpenEvent;
	hArrary[2] = data.hUsbCloseEvent;

	BOOL isClose = FALSE;
	while(isClose==FALSE)
	{
		switch(WaitForMultipleObjects(EVENT_COUNT, hArrary, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0:						// Exit Thread
			isClose = TRUE;
			::SendMessage(data.hWnd, WM_CLOSE, NULL, NULL);
			SetEvent(data.hWaitCloseEvent);
			break;

		case WAIT_OBJECT_0 + 1:					// Open Usb

			printf("usb device arriving...\n");

			CommControl::GetInstance()->ReopenUsb();

			if(g_usbDeviceListenerOpenCallback)
				g_usbDeviceListenerOpenCallback(g_usbDeviceListenerOpenCallbackParam);

			ResetEvent(data.hUsbOpenEvent);
			break;

		case WAIT_OBJECT_0 + 2:					// Close Usb

			printf("usb device removing...\n");

			CommControl::GetInstance()->CloseUsb();

			if(g_usbDeviceListenerCloseCallback)
				g_usbDeviceListenerCloseCallback(g_usbDeviceListenerCloseCallbackParam);

			ResetEvent(data.hUsbCloseEvent);
			break;
		}
	}
	

	return 0;
}

UINT UsbListenerWnd(LPVOID lpVoid)
{
	UsbListenerData &data = *(UsbListenerData*)lpVoid;

	CString str;
	
	srand(unsigned(time(0)));
	str.Format("UsbHidListenerWindowClass_%d", rand() * rand());
	
	data.hWnd = CreateUsbListenerWndClass(str);

	MSG msg; 
    int retVal;
    while((retVal=GetMessage(&msg, data.hWnd, 0, 0)) != 0)
    { 
        if (retVal==-1 || NULL==data.hWnd)	break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		Sleep(1);
    }
	
	return 0;
}

INT_PTR WINAPI UsbListenerWinProcCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = 1;
    static HDEVNOTIFY hDeviceNotify;
	static DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
	static GUID WceusbshGUID = {0x4D1E55B2,0xF16F,0x11CF,0x88,0xCB,0x00,0x11,0x11,0x00,0x00,0x30};
	static CString strName, strID;

    switch(message)
    {
    case WM_CREATE:
		ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
		NotificationFilter.dbcc_size		= sizeof(DEV_BROADCAST_DEVICEINTERFACE);
		NotificationFilter.dbcc_devicetype	= DBT_DEVTYP_DEVICEINTERFACE;
		NotificationFilter.dbcc_classguid	= WceusbshGUID;

		hDeviceNotify = RegisterDeviceNotification(hWnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
		if(NULL==hDeviceNotify)	ExitProcess(1);

        break;

    case WM_DEVICECHANGE:
    {
        PDEV_BROADCAST_DEVICEINTERFACE dbd = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

		/*
		+	dbcc_name	0x00148f4c "\\?\HID#Vid_880a&Pid_3501#7&283c5f26&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
		*/

		if(wParam==DBT_DEVICEARRIVAL || wParam==DBT_DEVICEREMOVECOMPLETE)
		{
			strID.Format(_T("VID_%04X&PID_%04X"), CommControl::GetInstance()->m_nUsbVendorId, CommControl::GetInstance()->m_nUsbProductId);
			strName = dbd->dbcc_name;
			strName.MakeUpper();

			if(strName.Find(strID) >=0)
			{
				if(wParam==DBT_DEVICEARRIVAL)
				{
					SetEvent(usbListenerData.hUsbOpenEvent);
				}
				else if(wParam==DBT_DEVICEREMOVECOMPLETE)
				{	
					SetEvent(usbListenerData.hUsbCloseEvent);
				}
			}
		}
        
    }
    break;

    case WM_CLOSE:
        UnregisterDeviceNotification(hDeviceNotify);
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        lRet = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 1;
}

HWND CreateUsbListenerWndClass(const char* WND_CLASS_NAME)
{
	WNDCLASSEX wndClass;
    wndClass.cbSize			= sizeof(WNDCLASSEX);
    wndClass.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wndClass.hInstance		= reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
    wndClass.lpfnWndProc	= reinterpret_cast<WNDPROC>(UsbListenerWinProcCallback);
    wndClass.cbClsExtra		= 0;
    wndClass.cbWndExtra		= 0;
    wndClass.hIcon			= LoadIcon(0,IDI_APPLICATION);
    wndClass.hbrBackground	= CreateSolidBrush(RGB(192,192,192));
    wndClass.hCursor		= LoadCursor(0, IDC_ARROW);
    wndClass.lpszClassName	= WND_CLASS_NAME;
    wndClass.lpszMenuName	= NULL;
    wndClass.hIconSm		= wndClass.hIcon;
	
    if(!RegisterClassEx(&wndClass))	return NULL;
	
	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE|WS_EX_APPWINDOW, WND_CLASS_NAME, "", 
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 0, 0, NULL, NULL, NULL, NULL);
	
	if(hWnd == NULL)
		return NULL;
	
	ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);
	
    return hWnd;
}
