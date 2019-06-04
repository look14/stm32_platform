#ifndef __USBHID_H
#define __USBHID_H

#include "typedefs.h"

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

#include <pthread.h>

typedef u8 (*pUsbHid_RecvCallback)(void* pParam, const u8 recvBuf[], u16 recvLen);
typedef void (*pUsbHid_RemovalCallback)(void* pParam);


class CUsbHid
{
public:
    /* Barrier implementation because Mac OSX doesn't have pthread_barrier.
     It also doesn't have clock_gettime(). So much for POSIX and SUSv2.
     This implementation came from Brent Priddy and was posted on
     StackOverflow. It is used with his permission. */
    typedef int pthread_barrierattr_t;
    typedef struct pthread_barrier {
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        int count;
        int trip_count;
    } pthread_barrier_t;
    
public:
	CUsbHid();
	virtual ~CUsbHid();
    
    virtual void Lock();
    virtual void Unlock();

    virtual BOOL Ready(void);
	virtual BOOL Close(void);
	virtual BOOL Open(u16 VID, u16 PID);
    
    virtual int Send(const u8 buf[], int nSize, int nTimeout=1000);
    virtual int Recv(u8 buf[], int nSize, int nTimeout=1000);
    
    void RegisterRecvCallback(pUsbHid_RecvCallback pCallback, void* pParam) {
        m_pUsbHid_RecvCallback = pCallback;
        m_pUsbHid_RecvParam = pParam;
    }
    void ResisterRemovalCallback(pUsbHid_RemovalCallback pCallback, void* pParam) {
        m_pUsbHid_RemovalCallback = pCallback;
        m_pUsbHid_RemovalParam = pParam;
    }

protected:
    int GetIntProperty(IOHIDDeviceRef device, CFStringRef key);
    int GetStringProperty(IOHIDDeviceRef device, CFStringRef prop, char *buf, size_t len);
    void TrimStringProperty(char *buf, size_t len);
    io_service_t IOHIDDeviceGetService(IOHIDDeviceRef device);
    BOOL OpenDevicePath();
    
    static void DeviceRemovalCallback(void *context, IOReturn result, void *sender);
    static void InputReportCallback(void *context, IOReturn result, void *sender, IOHIDReportType report_type,
                                    uint32_t report_id, uint8_t *report, CFIndex report_length);
    static void OutputReportCallback(void *context, IOReturn result, void *sender, IOHIDReportType report_type,
                                     uint32_t report_id, uint8_t *report, CFIndex report_length);
    static void PerformSignalCallback(void *context);
    static void* RecvThread(void* pParam);
    

protected:
    BOOL                    m_bUsbReady;
    IOHIDManagerRef         m_refHidManager;
    IOHIDDeviceRef          m_refHidDevice;
    CFStringRef             m_refRunLoopMode;
    CFRunLoopRef            m_refRunLoop;
    CFRunLoopSourceRef      m_refRunLoopSource;
    
	pUsbHid_RecvCallback	m_pUsbHid_RecvCallback;
    void*                   m_pUsbHid_RecvParam;
    pUsbHid_RemovalCallback m_pUsbHid_RemovalCallback;
    void*                   m_pUsbHid_RemovalParam;
    
    CFIndex                 m_nInputReportLen;
    u8*                     m_pInputReportBuf;
    
    u8                      m_recvBuf[65];
    int                     m_nReceivedLen;
    
    BOOL                    m_bThreadInited;
    BOOL                    m_bThreadRunning;
    pthread_t               m_threadRecv;
    pthread_mutex_t         m_mutexLock;
    
    pthread_cond_t          m_condWaitDone;
    pthread_mutex_t         m_mutexWaitDone;
    pthread_cond_t          m_condWaitExit;
    pthread_mutex_t         m_mutexWaitExit;
    
    pthread_cond_t          m_condReceived;
    pthread_mutex_t         m_mutexReceived;
    pthread_cond_t          m_condSended;
    pthread_mutex_t         m_mutexSended;

public:
    char        m_strDevicePath[512];
	char        m_strVendor[256];
	char        m_strProduct[256];
	char        m_strSerialNumber[256];
    u16         m_nVersionNumber;
};


#endif
