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
		SetEvent(m_hRecvExitEvent);    //�˳��߳�

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
	m_hRecvExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);        /* ���������߳��˳��¼�*/
	m_hConfirmExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (NULL==m_hRecvExitEvent || NULL==m_hReceivedEvent || NULL==m_hConfirmExitEvent) {
		return FALSE;
	}

	ResetEvent(m_hReceivedEvent);
	ResetEvent(m_hRecvExitEvent);                                    //�����߳�û���˳�
	ResetEvent(m_hConfirmExitEvent);

	HANDLE hThread    = NULL;
	DWORD  dwThreadID = 0;

	// ���������߳�
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
	GUID            HidGuid;			// ����һ��GUID�Ľṹ��HidGuid������HID�豸�Ľӿ���GUID��
	HDEVINFO        hDevInfoSet;		// ����һ��DEVINFO�ľ��hDevInfoSet�������ȡ�����豸��Ϣ���Ͼ����
	DWORD           MemberIndex;		// ����MemberIndex����ʾ��ǰ�������ڼ����豸��0��ʾ��һ���豸��
	BOOL            Result;				// ����һ��BOOL���������溯�������Ƿ񷵻سɹ�
	DWORD           RequiredSize;		// ����һ��RequiredSize�ı���������������Ҫ������ϸ��Ϣ�Ļ��峤�ȡ�
	//HANDLE          DevHandle;			// ����һ������������豸�ľ����           
	HIDD_ATTRIBUTES DevAttributes;		// ����һ��HIDD_ATTRIBUTES�Ľṹ������������豸�����ԡ�
	SP_DEVICE_INTERFACE_DATA DevInfoData;				// DevInfoData�����������豸�������ӿ���Ϣ
	PSP_DEVICE_INTERFACE_DETAIL_DATA    pDevDetailData;	// ����һ��ָ���豸��ϸ��Ϣ�Ľṹ��ָ��


	DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;
	DevInfoData.cbSize = sizeof(DevInfoData);
	DevAttributes.Size = sizeof(DevAttributes);

	// ����HidD_GetHidGuid������ȡHID�豸��GUID����������HidGuid�С�
	HidD_GetHidGuid(&HidGuid);
	hDevInfoSet = SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
	MemberIndex=0;

	while(1)
	{
		//����SetupDiEnumDeviceInterfaces���豸��Ϣ�����л�ȡ���ΪMemberIndex���豸��Ϣ��
		Result = SetupDiEnumDeviceInterfaces(hDevInfoSet, 0, &HidGuid, MemberIndex, &DevInfoData);
		if(Result==FALSE) break;

		MemberIndex++;

		//�����ȡ��Ϣ�ɹ����������ȡ���豸����ϸ��Ϣ���ڻ�ȡ�豸
		//��ϸ��Ϣʱ����Ҫ��֪��������ϸ��Ϣ��Ҫ���Ļ���������ͨ��
		//��һ�ε��ú���SetupDiGetDeviceInterfaceDetail����ȡ����ʱ
		//�ṩ�������ͳ��ȶ�ΪNULL�Ĳ��������ṩһ������������Ҫ���
		//�������ı���RequiredSize��
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &DevInfoData, NULL, 0, &RequiredSize, NULL);

		//Ȼ�󣬷���һ����СΪRequiredSize�����������������豸��ϸ��Ϣ��
		pDevDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);

		if(pDevDetailData==NULL) //����ڴ治�㣬��ֱ�ӷ��ء�
		{
			TRACE("�ڴ治��!");
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			return FALSE;
		}

		//������pDevDetailData��cbSizeΪ�ṹ��Ĵ�С��ע��ֻ�ǽṹ���С�����������滺��������
		pDevDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		//Ȼ���ٴε���SetupDiGetDeviceInterfaceDetail��������ȡ�豸����ϸ��Ϣ����ε�������ʹ�õĻ������Լ���������С��
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &DevInfoData, pDevDetailData, RequiredSize, &Required, NULL);

		m_strDevicePath=pDevDetailData->DevicePath;
		free(pDevDetailData);

		//�������ʧ�ܣ��������һ���豸��
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
			//��ȡ�豸�����Բ�������DevAttributes�ṹ����
			Result = HidD_GetAttributes(m_hDevHandle, &DevAttributes);

			//��ȡʧ�ܣ�������һ��
			if(Result==FALSE)  
			{
				//�رոոմ򿪵��豸
				CloseHandle(m_hDevHandle);
				m_hDevHandle = NULL;
				continue;
			}

			//�����ȡ�ɹ����������е�VID��PID�Լ��豸�汾����������Ҫ�Ľ��бȽϣ������һ�µĻ�����˵������������Ҫ�ҵ��豸��
			if( DevAttributes.VendorID==VID && //���VID���
				DevAttributes.ProductID==PID) //����PID���
					//if(DevAttributes.VersionNumber==StUsbID.m_dwPVN) //�����豸�汾�����
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
					
					//��ȡ�豸���Խṹ��
					PHIDP_PREPARSED_DATA  PreparsedData;
					HidD_GetPreparsedData(m_hDevHandle, &PreparsedData);
					HidP_GetCaps(PreparsedData, &m_Capabilities);

					//�ͷ���Դ
					HidD_FreePreparsedData(PreparsedData);

					//��ô��������Ҫ�ҵ��豸���ֱ�ʹ�ö�д��ʽ��֮������������,����ѡ��Ϊ�첽���ʷ�ʽ,����ʽ���豸
					m_hUSBRead = CreateFile(m_strDevicePath,
						GENERIC_READ,
						FILE_SHARE_READ|FILE_SHARE_WRITE, 
						(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
						NULL);

					if (INVALID_HANDLE_VALUE==m_hUSBRead) {
						TRACE("�����ʴ�HidUsb�豸ʧ��......!\n");
					}
					else {
						TRACE("�����ʴ�HidUsb�豸�ɹ�......!\n");
					}

					//д��ʽ���豸
					m_hUSBWrite = CreateFile(m_strDevicePath,
						GENERIC_WRITE,
						FILE_SHARE_READ|FILE_SHARE_WRITE, 
						(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
						NULL);

					if (INVALID_HANDLE_VALUE==m_hUSBWrite) {
						TRACE("д���ʴ�HidUsb�豸ʧ��......!\n");
					}
					else {
						TRACE("д���ʴ�HidUsb�豸�ɹ�......!\n");
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
			continue;	//�����ʧ�ܣ��������һ���豸
		}
	}


	//����SetupDiDestroyDeviceInfoList���������豸��Ϣ����
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

	//HID���ͱ����һ���ֽڱ���Ϊ0, ���Է����ܳ���Ϊ0x41=65    
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
			break; // �߳��˳�
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

			// ͨ��GetOverlappedResult��������ȡʵ�ʶ�ȡ�����ֽ�����
			GetOverlappedResult(pArg->m_hUSBRead, &pArg->m_ovUSBRead, &dwRecvBytes, TRUE);

			if (dwRecvBytes && pArg->m_szUSBRecvBuf)
			{
				memcpy(pArg->m_szUSBRecvBuf, &szRecvBuf[1], pArg->m_nUSBRecvBufSize);
				pArg->m_nRecvLength = (UINT)(dwRecvBytes-1); // Ĭ�Ϸ���65���ֽڣ�����Ҫ��1
				dwRecvBytes = 0;

				// ��������Ϣ�Ž����¸��������������Ҫ����ͬ���¼���
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