#include "stdafx.h"
#include "UsbHid.h"

CUsbHid::CUsbHid()
{
	m_hDevHandle		= NULL;
	m_ovUSBRead.hEvent	= NULL;
	m_ovUSBWrite.hEvent = NULL;
	m_pOwner			= NULL;
	m_hUSBWrite			= NULL;
	m_hUSBWrite			= NULL;
	m_hUSBRead			= NULL;
	m_hRecvExitEvent	= NULL;
	m_hConfirmExitEvent = NULL;
	m_nRecvLength		= 0;
	m_bInit				= FALSE;
	m_szUSBRecvBuf		= NULL;
	m_hReceivedEvent	= NULL;
	m_nUSBRecvBufSize	= 64;
	m_nUSBSendBufSize	= 64;
	m_nRecvTimeout      = 2000;
	m_nSendTimeout      = 2000;
	m_pUsbHid_RecvCallback = NULL;
	m_pUsbHid_RecvParam = NULL;

	memset(m_strVendor, 0, sizeof(m_strVendor));
	memset(m_strProduct, 0, sizeof(m_strProduct));
	memset(m_strSerialNumber, 0, sizeof(m_strSerialNumber));

	::InitializeCriticalSection(&m_csUsbHid);
}

CUsbHid::~CUsbHid()
{
	Close();
	::DeleteCriticalSection(&m_csUsbHid);
}

void CUsbHid::Lock()
{
	::EnterCriticalSection(&m_csUsbHid);
}

void CUsbHid::Unlock()
{
	::LeaveCriticalSection(&m_csUsbHid);
}

BOOL CUsbHid::Close(void)
{
	m_bInit = FALSE;

	memset(m_strVendor, 0, sizeof(m_strVendor));
	memset(m_strProduct, 0, sizeof(m_strProduct));
	memset(m_strSerialNumber, 0, sizeof(m_strSerialNumber));

	if (m_hRecvExitEvent) {
		SetEvent(m_hRecvExitEvent);    //退出线程

		if(m_hConfirmExitEvent)
			WaitForSingleObject(m_hConfirmExitEvent, 1000);

		CloseHandle(m_hRecvExitEvent);
		m_hRecvExitEvent = NULL;
		//	Sleep(100);
	}

	if(m_hConfirmExitEvent) {
		CloseHandle(m_hConfirmExitEvent);
		m_hConfirmExitEvent = NULL;
	}

	if (m_hUSBWrite) {
		CloseHandle(m_hUSBWrite);
		m_hUSBWrite = NULL;
	}

	if (m_hUSBRead) {
		CloseHandle(m_hUSBRead);
		m_hUSBRead = NULL;
	}

	if (m_ovUSBRead.hEvent) {
		CloseHandle(m_ovUSBRead.hEvent);
		m_ovUSBRead.hEvent = NULL;
	}

	if (m_ovUSBWrite.hEvent) {
		CloseHandle(m_ovUSBWrite.hEvent);
		m_ovUSBWrite.hEvent = NULL;
	}

	if(m_hReceivedEvent) {
		CloseHandle(m_hReceivedEvent);
		m_hReceivedEvent = NULL;
	}

	if (m_pOwner) {
		m_pOwner = NULL;
	}

	if (m_szUSBRecvBuf) {
		delete []m_szUSBRecvBuf;
		m_szUSBRecvBuf = NULL;
	}

	if(m_hDevHandle) {
		CloseHandle(m_hDevHandle);
		m_hDevHandle = NULL;
	}

	return TRUE;
}

BOOL CUsbHid::Ready(void)
{
	return m_bInit;
}

BOOL CUsbHid::CreateThreadAndEvent(void)
{
	m_ovUSBRead.Offset     = 0;
	m_ovUSBRead.OffsetHigh = 0;
	m_ovUSBRead.hEvent     = CreateEvent(NULL,TRUE,FALSE,NULL);

	if (NULL==m_ovUSBRead.hEvent) {
		return FALSE;
	}
	else {
		ResetEvent(m_ovUSBRead.hEvent);
	}


	m_ovUSBWrite.Offset     = 0;
	m_ovUSBWrite.OffsetHigh = 0;
	m_ovUSBWrite.hEvent     = CreateEvent(NULL,TRUE,FALSE,NULL);

	if (NULL==m_ovUSBWrite.hEvent) {
		return FALSE;
	}
	else {
		ResetEvent(m_ovUSBWrite.hEvent);
	}


	m_hReceivedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hRecvExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);        /* 创建接收线程退出事件*/
	m_hConfirmExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (NULL==m_hRecvExitEvent || NULL==m_hReceivedEvent || NULL==m_hConfirmExitEvent) {
		return FALSE;
	}

	ResetEvent(m_hReceivedEvent);
	ResetEvent(m_hRecvExitEvent);                                    //设置线程没有退出
	ResetEvent(m_hConfirmExitEvent);

	HANDLE hThread    = NULL;
	DWORD  dwThreadID = 0;

	// 创建接收线程
	hThread=CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RecvThread, this, 0, &dwThreadID);
	if(NULL==hThread) {
		return FALSE;
	}

	CloseHandle(hThread);
	hThread = NULL;

	return TRUE;
}

void CUsbHid::TrimStringProperty(char *buf, size_t len)
{
	int i, index;
	char strTmp[256];

	for(i=0, index=0; i<len; i++) {
		if(buf[i] != 0) {
			strTmp[index++] = buf[i];
		}
	}
	strTmp[index++] = 0;
	memcpy(buf, strTmp, index);
}

BOOL CUsbHid::Open(CWnd *pPortOwner, DWORD VID, DWORD PID)
{
	m_pOwner = pPortOwner;

	/*if (Ready()) {
		Close();
	}*/

	Close();

	if (!m_szUSBRecvBuf) {
		m_szUSBRecvBuf = new UCHAR[m_nUSBRecvBufSize];
		memset(m_szUSBRecvBuf, 0, m_nUSBRecvBufSize);
	}

	ULONG           Required;
	GUID            HidGuid;			// 定义一个GUID的结构体HidGuid来保存HID设备的接口类GUID。
	HDEVINFO        hDevInfoSet;		// 定义一个DEVINFO的句柄hDevInfoSet来保存获取到的设备信息集合句柄。
	DWORD           MemberIndex;		// 定义MemberIndex，表示当前搜索到第几个设备，0表示第一个设备。
	BOOL            Result;				// 定义一个BOOL变量，保存函数调用是否返回成功
	DWORD           RequiredSize;		// 定义一个RequiredSize的变量，用来接收需要保存详细信息的缓冲长度。
	//HANDLE          DevHandle;			// 定义一个用来保存打开设备的句柄。           
	HIDD_ATTRIBUTES DevAttributes;		// 定义一个HIDD_ATTRIBUTES的结构体变量，保存设备的属性。
	SP_DEVICE_INTERFACE_DATA DevInfoData;				// DevInfoData，用来保存设备的驱动接口信息
	PSP_DEVICE_INTERFACE_DETAIL_DATA    pDevDetailData;	// 定义一个指向设备详细信息的结构体指针


	DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;
	DevInfoData.cbSize = sizeof(DevInfoData);
	DevAttributes.Size = sizeof(DevAttributes);

	// 调用HidD_GetHidGuid函数获取HID设备的GUID，并保存在HidGuid中。
	HidD_GetHidGuid(&HidGuid);
	hDevInfoSet = SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
	MemberIndex=0;

	while(1)
	{
		//调用SetupDiEnumDeviceInterfaces在设备信息集合中获取编号为MemberIndex的设备信息。
		Result = SetupDiEnumDeviceInterfaces(hDevInfoSet, 0, &HidGuid, MemberIndex, &DevInfoData);
		if(Result==FALSE) break;

		MemberIndex++;

		//如果获取信息成功，则继续获取该设备的详细信息。在获取设备
		//详细信息时，需要先知道保存详细信息需要多大的缓冲区，这通过
		//第一次调用函数SetupDiGetDeviceInterfaceDetail来获取。这时
		//提供缓冲区和长度都为NULL的参数，并提供一个用来保存需要多大
		//缓冲区的变量RequiredSize。
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &DevInfoData, NULL, 0, &RequiredSize, NULL);

		//然后，分配一个大小为RequiredSize缓冲区，用来保存设备详细信息。
		pDevDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);

		if(pDevDetailData==NULL) //如果内存不足，则直接返回。
		{
			TRACE("内存不足!");
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			return FALSE;
		}

		//并设置pDevDetailData的cbSize为结构体的大小（注意只是结构体大小，不包括后面缓冲区）。
		pDevDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		//然后再次调用SetupDiGetDeviceInterfaceDetail函数来获取设备的详细信息。这次调用设置使用的缓冲区以及缓冲区大小。
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &DevInfoData, pDevDetailData, RequiredSize, &Required, NULL);

		m_strDevicePath=pDevDetailData->DevicePath;
		free(pDevDetailData);

		//如果调用失败，则查找下一个设备。
		if(Result==FALSE) continue;

		m_hDevHandle = CreateFile(m_strDevicePath, 
			0,
			FILE_SHARE_READ|FILE_SHARE_WRITE, 
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if(m_hDevHandle != INVALID_HANDLE_VALUE)
		{
			//获取设备的属性并保存在DevAttributes结构体中
			Result = HidD_GetAttributes(m_hDevHandle, &DevAttributes);

			//获取失败，查找下一个
			if(Result==FALSE)  
			{
				//关闭刚刚打开的设备
				CloseHandle(m_hDevHandle);
				m_hDevHandle = NULL;
				continue;
			}

			//如果获取成功，则将属性中的VID、PID以及设备版本号与我们需要的进行比较，如果都一致的话，则说明它就是我们要找的设备。
			if( DevAttributes.VendorID==VID && //如果VID相等
				DevAttributes.ProductID==PID) //并且PID相等
					//if(DevAttributes.VersionNumber==StUsbID.m_dwPVN) //并且设备版本号相等
				{
					DevBroadcastDeviceInterface.dbcc_size       = sizeof(DevBroadcastDeviceInterface);
					DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
					DevBroadcastDeviceInterface.dbcc_classguid  = HidGuid;
					
					HidD_GetManufacturerString(m_hDevHandle, m_strVendor, sizeof(m_strVendor));
					HidD_GetProductString(m_hDevHandle, m_strProduct, sizeof(m_strProduct));
					HidD_GetSerialNumberString(m_hDevHandle, m_strSerialNumber, sizeof(m_strSerialNumber));

					TrimStringProperty(m_strVendor, sizeof(m_strVendor));
					TrimStringProperty(m_strProduct, sizeof(m_strProduct));
					TrimStringProperty(m_strSerialNumber, sizeof(m_strSerialNumber));
					
					//获取设备属性结构体
					PHIDP_PREPARSED_DATA  PreparsedData;
					HidD_GetPreparsedData(m_hDevHandle, &PreparsedData);
					HidP_GetCaps(PreparsedData, &m_Capabilities);

					//释放资源
					HidD_FreePreparsedData(PreparsedData);

					//那么就是我们要找的设备，分别使用读写方式打开之，并保存其句柄,并且选择为异步访问方式,读方式打开设备
					m_hUSBRead = CreateFile(m_strDevicePath,
						GENERIC_READ,
						FILE_SHARE_READ|FILE_SHARE_WRITE, 
						(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
						NULL);

					if (INVALID_HANDLE_VALUE==m_hUSBRead) {
						TRACE("读访问打开HidUsb设备失败......!\n");
					}
					else {
						TRACE("读访问打开HidUsb设备成功......!\n");
					}

					//写方式打开设备
					m_hUSBWrite = CreateFile(m_strDevicePath,
						GENERIC_WRITE,
						FILE_SHARE_READ|FILE_SHARE_WRITE, 
						(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
						NULL);

					if (INVALID_HANDLE_VALUE==m_hUSBWrite) {
						TRACE("写访问打开HidUsb设备失败......!\n");
					}
					else {
						TRACE("写访问打开HidUsb设备成功......!\n");
					}

					if (m_hUSBRead  == INVALID_HANDLE_VALUE && m_hUSBWrite == INVALID_HANDLE_VALUE) {
						return FALSE;
					}

					if (!CreateThreadAndEvent()) {
						return FALSE;
					}

					m_bInit = TRUE;

					if(pPortOwner) 
						RegisterHIDDevice(pPortOwner,HidGuid);

					return TRUE;
				}
		}
		else {
			m_hDevHandle = NULL;
			continue;	//如果打开失败，则查找下一个设备
		}
	}


	//调用SetupDiDestroyDeviceInfoList函数销毁设备信息集合
	SetupDiDestroyDeviceInfoList(hDevInfoSet);

	return FALSE;

}
UINT CUsbHid::Send(const UCHAR *pSendBytes, UINT nSendLen)
{
	if (NULL == pSendBytes || 0==nSendLen) {
		return 0;
	}

	if (m_hUSBWrite==INVALID_HANDLE_VALUE || m_hUSBWrite==NULL) {
		return 0;
	}

	UCHAR szSendBuf[65] = {0};
	DWORD dwSendBytes   = 0;
	INT  rt = 0;

	//szSendBuf[0] = 0x3F;

	//HID发送报告第一个字节必须为0, 所以发送总长度为0x41=65    
	memcpy(&szSendBuf[1], pSendBytes, m_nUSBSendBufSize);

	rt = WriteFile(m_hUSBWrite,
		szSendBuf,
		m_Capabilities.OutputReportByteLength,
		NULL,
		&m_ovUSBWrite);

	if(WaitForSingleObject(m_ovUSBWrite.hEvent, m_nSendTimeout)==WAIT_OBJECT_0) {
		GetOverlappedResult(m_hUSBWrite, &m_ovUSBWrite, &dwSendBytes, FALSE);
		ResetEvent(m_ovUSBWrite.hEvent);
	}
	else {
		ResetEvent(m_ovUSBWrite.hEvent);
		return 0;
	}

	return (UINT)(dwSendBytes-1);
}

UINT CUsbHid::Recv(UCHAR *pRecvBytes)
{
	UINT nRecvLength = 0;

	if (NULL == pRecvBytes) {
		return 0;
	}

	if (m_hUSBRead==INVALID_HANDLE_VALUE || m_hUSBRead==NULL) {
		return 0;
	}  

	if(WaitForSingleObject(m_hReceivedEvent, m_nRecvTimeout)==WAIT_OBJECT_0) {
		ResetEvent(m_hReceivedEvent);
		nRecvLength   = m_nRecvLength;
		m_nRecvLength = 0;

		memcpy(pRecvBytes, m_szUSBRecvBuf, m_nUSBRecvBufSize);
	}
	else {
		nRecvLength = 0;
	}

	return nRecvLength;
}

DWORD CUsbHid::RecvThread(LPVOID lpArg)
{
	CUsbHid *pArg=(CUsbHid *)lpArg;

	UCHAR szRecvBuf[65];
	DWORD dwRecvBytes   = 0;

	HANDLE hEventArr[2];

	DWORD dRet;

	hEventArr[0] = pArg->m_hRecvExitEvent;
	hEventArr[1] = pArg->m_ovUSBRead.hEvent;

	while(1)
	{
		if(WaitForSingleObject(pArg->m_hRecvExitEvent, 0)==WAIT_OBJECT_0) {
			break; // 线程退出
		}

		if (pArg->Ready())
		{
			memset(szRecvBuf,0,sizeof(szRecvBuf));

			ReadFile(pArg->m_hUSBRead,
				szRecvBuf,
				pArg->m_Capabilities.InputReportByteLength,
				NULL,
				&pArg->m_ovUSBRead);

			//WaitForSingleObject(pArg->m_ovUSBRead.hEvent, INFINITE);
			//ResetEvent(pArg->m_ovUSBRead.hEvent);

			dRet = WaitForMultipleObjects(2, hEventArr, FALSE, INFINITE);

			if(WAIT_OBJECT_0 == dRet) {
				SetEvent(pArg->m_hConfirmExitEvent);
				break;
			}
			else if(WAIT_OBJECT_0+1 == dRet) {
				ResetEvent(pArg->m_ovUSBRead.hEvent);
			}
			else {
				continue;
			}

			// 通过GetOverlappedResult函数来获取实际读取到的字节数。
			GetOverlappedResult(pArg->m_hUSBRead, &pArg->m_ovUSBRead, &dwRecvBytes, TRUE);

			if (dwRecvBytes && pArg->m_szUSBRecvBuf)
			{
				memcpy(pArg->m_szUSBRecvBuf, &szRecvBuf[1], pArg->m_nUSBRecvBufSize);
				pArg->m_nRecvLength = (UINT)(dwRecvBytes-1); // 默认返回65个字节，所以要减1
				dwRecvBytes = 0;

				// 完成这个消息才进行下个操作，因而不需要加上同步事件。
				SetEvent(pArg->m_hReceivedEvent);

				if(pArg->m_pUsbHid_RecvCallback) {
					pArg->m_pUsbHid_RecvCallback(pArg->m_pUsbHid_RecvParam, pArg->m_szUSBRecvBuf, pArg->m_nRecvLength);
				}

			}

		}
		//Sleep(1);
	}

	return 0;
}


void CUsbHid::GetDevicePath(CString &str)
{
	str = m_strDevicePath;
}

void  CUsbHid::RegisterHIDDevice(CWnd *pPortOwner, GUID HidGuid)
{
	DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;
	HDEVNOTIFY DeviceNotificationHandle;

	DevBroadcastDeviceInterface.dbcc_size       = sizeof(DevBroadcastDeviceInterface);
	DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	DevBroadcastDeviceInterface.dbcc_classguid  = HidGuid;

	DeviceNotificationHandle = RegisterDeviceNotification(pPortOwner->m_hWnd, &DevBroadcastDeviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);
}