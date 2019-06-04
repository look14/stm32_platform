#include "stdafx.h"
#include "UsbHid.h"

#include <memory.h>
#include <locale.h>
#include <sys/time.h>
#include <unistd.h>
#include <dlfcn.h>

CUsbHid::CUsbHid()
{
    m_bUsbReady         = FALSE;
    m_bThreadInited     = FALSE;
    m_bThreadRunning    = FALSE;
    m_refHidManager     = NULL;
    m_refHidDevice      = NULL;
    m_nVersionNumber    = 0;
    m_nInputReportLen   = 0;
    m_pInputReportBuf   = NULL;
    
    m_refRunLoopMode    = NULL;
    m_refRunLoop        = NULL;
    m_refRunLoopSource  = NULL;
 
    m_pUsbHid_RecvCallback = NULL;
    m_pUsbHid_RecvParam = NULL;
    m_pUsbHid_RemovalCallback = NULL;
    m_pUsbHid_RemovalParam = NULL;
    
    m_nReceivedLen = 0;
    memset(m_recvBuf, 0, sizeof(m_recvBuf));
    
    memset(m_strDevicePath, 0, sizeof(m_strDevicePath));
	memset(m_strVendor, 0, sizeof(m_strVendor));
	memset(m_strProduct, 0, sizeof(m_strProduct));
	memset(m_strSerialNumber, 0, sizeof(m_strSerialNumber));
}

CUsbHid::~CUsbHid()
{
	Close();
}

int CUsbHid::GetIntProperty(IOHIDDeviceRef device, CFStringRef key)
{
    CFTypeRef ref;
    int value;
    
    ref = IOHIDDeviceGetProperty(device, key);
    if (ref) {
        if (CFGetTypeID(ref) == CFNumberGetTypeID()) {
            CFNumberGetValue((CFNumberRef) ref, kCFNumberSInt32Type, &value);
            return value;
        }
    }

    return 0;
}

int CUsbHid::GetStringProperty(IOHIDDeviceRef device, CFStringRef prop, char *buf, size_t len)
{
    CFStringRef str;
    
    if (!len)
        return 0;
    
    str = (CFStringRef)IOHIDDeviceGetProperty(device, prop);
    
    buf[0] = 0;
    
    if (str) {
        CFIndex str_len = CFStringGetLength(str);
        CFRange range;
        CFIndex used_buf_len;
        CFIndex chars_copied;
        
        len--;
        
        range.location = 0;
        range.length = ((size_t)str_len > len)? len: (size_t)str_len;
        chars_copied = CFStringGetBytes(str,
                                        range,
                                        kCFStringEncodingUTF32LE,
                                        (char)'?',
                                        FALSE,
                                        (UInt8*)buf,
                                        len * sizeof(wchar_t),
                                        &used_buf_len);
        
        if (chars_copied == len)
            buf[len] = 0; /* len is decremented above */
        else
            buf[chars_copied] = 0;
        
        return 0;
    }
    
    return -1;
}

io_service_t CUsbHid::IOHIDDeviceGetService(IOHIDDeviceRef device)
{
    typedef io_service_t (*tIOHIDDeviceGetService)(IOHIDDeviceRef device);
    
    static void *iokit_framework = NULL;
    static tIOHIDDeviceGetService dynamic_IOHIDDeviceGetService = NULL;
    
    /* Use dlopen()/dlsym() to get a pointer to IOHIDDeviceGetService() if it exists.
     * If any of these steps fail, dynamic_IOHIDDeviceGetService will be left NULL
     * and the fallback method will be used.
     */
    if (iokit_framework == NULL) {
        iokit_framework = dlopen("/System/Library/IOKit.framework/IOKit", RTLD_LAZY);
        
        if (iokit_framework != NULL)
            dynamic_IOHIDDeviceGetService = (tIOHIDDeviceGetService)dlsym(iokit_framework, "IOHIDDeviceGetService");
    }
    
    if (dynamic_IOHIDDeviceGetService != NULL) {
        /* Running on OS X 10.6 and above: IOHIDDeviceGetService() exists */
        return dynamic_IOHIDDeviceGetService(device);
    }
    else
    {
        /* Running on OS X 10.5: IOHIDDeviceGetService() doesn't exist.
         *
         * Be naughty and pull the service out of the IOHIDDevice.
         * IOHIDDevice is an opaque struct not exposed to applications, but its
         * layout is stable through all available versions of OS X.
         * Tested and working on OS X 10.5.8 i386, x86_64, and ppc.
         */
        struct IOHIDDevice_internal {
            /* The first field of the IOHIDDevice struct is a
             * CFRuntimeBase (which is a private CF struct).
             *
             * a, b, and c are the 3 fields that make up a CFRuntimeBase.
             * See http://opensource.apple.com/source/CF/CF-476.18/CFRuntime.h
             *
             * The second field of the IOHIDDevice is the io_service_t we're looking for.
             */
            uintptr_t a;
            uint8_t b[4];
#if __LP64__
            uint32_t c;
#endif
            io_service_t service;
        };
        struct IOHIDDevice_internal *tmp = (struct IOHIDDevice_internal *)device;
        
        return tmp->service;
    }
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

void CUsbHid::Lock()
{
    pthread_mutex_lock(&m_mutexLock);
}

void CUsbHid::Unlock()
{
    pthread_mutex_unlock(&m_mutexLock);
}

BOOL CUsbHid::Ready(void)
{
    return m_bUsbReady;
}

BOOL CUsbHid::Close(void)
{
    if(m_bUsbReady)
    {
        /* Disconnect the report callback before close. */
        IOHIDDeviceRegisterInputReportCallback(m_refHidDevice, m_pInputReportBuf, m_nInputReportLen, NULL, this);
        IOHIDDeviceRegisterRemovalCallback(m_refHidDevice, NULL, this);
        IOHIDDeviceUnscheduleFromRunLoop(m_refHidDevice, m_refRunLoop, m_refRunLoopMode);
        IOHIDDeviceScheduleWithRunLoop(m_refHidDevice, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
        
        /* Wake up the run thread's event loop so that the thread can exit. */
        CFRunLoopSourceSignal(m_refRunLoopSource);
        CFRunLoopWakeUp(m_refRunLoop);
        
        if(m_bThreadRunning) {
            /* Notify the thread that it can shut down now. */
            pthread_mutex_lock(&m_mutexWaitExit);
            pthread_cond_wait(&m_condWaitExit, &m_mutexWaitExit);
            pthread_mutex_unlock(&m_mutexWaitExit);
        }

        /* Wait for thread to end. */
        pthread_join(m_threadRecv, NULL);

        m_bUsbReady = FALSE;
    }
    
    if(m_bThreadInited) {
        m_bThreadInited = FALSE;
        
        /* Clean up the thread objects */
        pthread_mutex_destroy(&m_mutexLock);
        pthread_cond_destroy(&m_condWaitDone);
        pthread_mutex_destroy(&m_mutexWaitDone);
        pthread_cond_destroy(&m_condWaitExit);
        pthread_mutex_destroy(&m_mutexWaitExit);
        
        pthread_cond_destroy(&m_condReceived);
        pthread_mutex_destroy(&m_mutexReceived);
        pthread_cond_destroy(&m_condSended);
        pthread_mutex_destroy(&m_mutexSended);
        
    }
    
    if(m_refRunLoop) {
        m_refRunLoop = NULL;
    }
    
    if(m_refRunLoopMode) {
        CFRelease(m_refRunLoopMode);
        m_refRunLoopMode = NULL;
    }
    
    if(m_refRunLoopSource) {
        CFRelease(m_refRunLoopSource);
        m_refRunLoopSource  = NULL;
    }
    
    if(m_pInputReportBuf) {
        delete[] m_pInputReportBuf;
        m_pInputReportBuf = NULL;
    }
    
    if(m_refHidDevice) {
        IOHIDDeviceClose(m_refHidDevice, kIOHIDOptionsTypeSeizeDevice);
        CFRelease(m_refHidDevice);
        m_refHidDevice = NULL;
    }
    
    if(m_refHidManager) {
        IOHIDManagerClose(m_refHidManager, kIOHIDOptionsTypeNone);
        CFRelease(m_refHidManager);
        m_refHidManager = NULL;
    }

	return TRUE;
}

BOOL CUsbHid::Open(u16 VID, u16 PID)
{
    SInt32 res;
    CFIndex num_devices;
    int i;
    BOOL bRet = FALSE;
    
    Close();
    
    /* Initialize all the HID Manager Objects */
    m_refHidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if(!m_refHidManager)
        return bRet;
    
    IOHIDManagerSetDeviceMatching(m_refHidManager, NULL);
    IOHIDManagerScheduleWithRunLoop(m_refHidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    
    /* give the IOHIDManager a chance to update itself */
    do {
        res = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, FALSE);
    } while(res != kCFRunLoopRunFinished && res != kCFRunLoopRunTimedOut);
    
    /* Get a list of the Devices */
    IOHIDManagerSetDeviceMatching(m_refHidManager, NULL);
    CFSetRef device_set = IOHIDManagerCopyDevices(m_refHidManager);
    
    /* Convert the list into a C array so we can iterate easily. */
    num_devices = CFSetGetCount(device_set);
    IOHIDDeviceRef *device_array = (IOHIDDeviceRef *)calloc(num_devices, sizeof(IOHIDDeviceRef));
    CFSetGetValues(device_set, (const void **) device_array);
    
    unsigned short dev_vid;
    unsigned short dev_pid;
    IOHIDDeviceRef dev;
    
    /* Iterate over each device, making an entry for it. */
    for (i = 0; i < num_devices; i++)
    {
        if (!(dev = device_array[i]))
            continue;
        
        dev_vid = GetIntProperty(dev, CFSTR(kIOHIDVendorIDKey));
        dev_pid = GetIntProperty(dev, CFSTR(kIOHIDProductIDKey));
        
        /* Check the VID/PID against the arguments */
        if (VID == dev_vid && PID == dev_pid)
        {
            /* Fill in the path (IOService plane) */
            if(KERN_SUCCESS != IORegistryEntryGetPath(IOHIDDeviceGetService(dev), kIOServicePlane, m_strDevicePath))
                memset(m_strDevicePath, 0, sizeof(m_strDevicePath));
            
            /* Serial Number */
            GetStringProperty(dev, CFSTR(kIOHIDSerialNumberKey), m_strSerialNumber, sizeof(m_strSerialNumber));
            
            /* Manufacturer and Product strings */
            GetStringProperty(dev, CFSTR(kIOHIDManufacturerKey), m_strVendor, sizeof(m_strVendor));
            GetStringProperty(dev, CFSTR(kIOHIDProductKey), m_strProduct, sizeof(m_strProduct));
            
            TrimStringProperty(m_strVendor, sizeof(m_strVendor));
            TrimStringProperty(m_strProduct, sizeof(m_strProduct));
            TrimStringProperty(m_strSerialNumber, sizeof(m_strSerialNumber));
            
            /* Release Number */
            m_nVersionNumber = GetIntProperty(dev, CFSTR(kIOHIDVersionNumberKey));
            
            bRet = OpenDevicePath();
            break;
        }
    }
    
    free(device_array);
    CFRelease(device_set);

	return bRet;
}

BOOL CUsbHid::OpenDevicePath()
{
    io_registry_entry_t entry = MACH_PORT_NULL;
    
    /* Get the IORegistry entry for the given path */
    entry = IORegistryEntryFromPath(kIOMasterPortDefault, m_strDevicePath);
    if (entry == MACH_PORT_NULL)
        return FALSE;
    
    /* Create an IOHIDDevice for the entry */
    m_refHidDevice = IOHIDDeviceCreate(kCFAllocatorDefault, entry);
    if (m_refHidDevice == NULL)
        return FALSE;
    
    /* Open the IOHIDDevice */
    if(kIOReturnSuccess==IOHIDDeviceOpen(m_refHidDevice, kIOHIDOptionsTypeSeizeDevice))
    {
        char str[32];
        
        /* Create the buffers for receiving data */
        m_nInputReportLen = (CFIndex)GetIntProperty(m_refHidDevice, CFSTR(kIOHIDMaxInputReportSizeKey));
        m_pInputReportBuf = new u8[m_nInputReportLen];
        
        /* Create the Run Loop Mode for this device.
         printing the reference seems to work. */
        sprintf(str, "HIDAPI_%p", m_refHidDevice);
        m_refRunLoopMode = CFStringCreateWithCString(NULL, str, kCFStringEncodingASCII);
     
        /* Attach the device to a Run Loop */
        IOHIDDeviceRegisterInputReportCallback(m_refHidDevice, m_pInputReportBuf, m_nInputReportLen, &InputReportCallback, this);
        IOHIDDeviceRegisterRemovalCallback(m_refHidDevice, DeviceRemovalCallback, this);
        
        /* Thread objects */
        pthread_mutex_init(&m_mutexLock, NULL);
        pthread_cond_init(&m_condWaitDone, NULL);
        pthread_mutex_init(&m_mutexWaitDone, NULL);
        pthread_cond_init(&m_condWaitExit, NULL);
        pthread_mutex_init(&m_mutexWaitExit, NULL);
        
        pthread_cond_init(&m_condReceived, NULL);
        pthread_mutex_init(&m_mutexReceived, NULL);
        pthread_cond_init(&m_condSended, NULL);
        pthread_mutex_init(&m_mutexSended, NULL);
        
        m_bThreadInited = TRUE;
        
        m_bUsbReady = TRUE;
        
        /* Start the read thread */
        pthread_create(&m_threadRecv, NULL, RecvThread, this);
        
        if(FALSE==m_bThreadRunning) {
            /* Wait here for the thread to be initialized. */
            pthread_mutex_lock(&m_mutexWaitDone);
            pthread_cond_wait(&m_condWaitDone, &m_mutexWaitDone);
            pthread_mutex_unlock(&m_mutexWaitDone);
        }
        
        IOObjectRelease(entry);
        return TRUE;
    }
    
    IOObjectRelease(entry);

    return FALSE;
}

int CUsbHid::Send(const u8 buf[], int nSize, int nTimeout)
{
    u8 sendBuf[65];
    int nSendLen;
    int nLen = -1;
    int nRes;
    
    if(FALSE==m_bUsbReady)
        return -1;
    
    sendBuf[0] = 0;
    memcpy(&sendBuf[1], buf, nSize);
    nSendLen = nSize;
    
#if 0 // This method asynchronously has system error, so disable it. 
    if(nTimeout > 0)
    {
        struct timespec ts;
        struct timeval tv;
        
        gettimeofday(&tv, NULL);
        TIMEVAL_TO_TIMESPEC(&tv, &ts);
        ts.tv_sec += nTimeout / 1000;
        ts.tv_nsec += (nTimeout % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000L;
        }
        
        nRes = IOHIDDeviceSetReportWithCallback(m_refHidDevice, kIOHIDReportTypeOutput, sendBuf[0], &sendBuf[1], nSendLen, nTimeout, OutputReportCallback, this);
        if(kIOReturnSuccess==nRes)
        {
            pthread_mutex_lock(&m_mutexSended);
            nRes = pthread_cond_timedwait(&m_condSended, &m_mutexSended, &ts);
    
            if(0==nRes)
                nLen = nSize;
        
            else if(ETIMEDOUT==nRes)
                nLen = 0;
    
            pthread_mutex_unlock(&m_mutexSended);
        }
    }
    else
#endif
    {
        nRes = IOHIDDeviceSetReport(m_refHidDevice, kIOHIDReportTypeOutput, sendBuf[0], &sendBuf[1], nSendLen);
        if(kIOReturnSuccess==nRes)
            nLen = nSize;
    }
    
    
    return nLen;
}

int CUsbHid::Recv(u8 buf[], int nSize, int nTimeout)
{
    int nLen = -1;
    int nRes;
    struct timespec ts;
    
    if(FALSE==m_bUsbReady)
        return -1;
    
    if(nTimeout > 0) {
        struct timeval tv;
        
        gettimeofday(&tv, NULL);
        TIMEVAL_TO_TIMESPEC(&tv, &ts);
        ts.tv_sec += nTimeout / 1000;
        ts.tv_nsec += (nTimeout % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000L;
        }
    }
    
    pthread_mutex_lock(&m_mutexReceived);
    nRes = pthread_cond_timedwait(&m_condReceived, &m_mutexReceived, &ts);
    
    if(0==nRes) {
        nLen = __min(m_nReceivedLen, nSize);
        memcpy(buf, m_recvBuf, nLen);
    }
    else if(ETIMEDOUT==nRes) {
        nLen = 0;
    }
    else {
        nLen = -1;
    }
    
    pthread_mutex_unlock(&m_mutexReceived);
    
    return nLen;
}

void CUsbHid::DeviceRemovalCallback(void *context, IOReturn result, void *sender)
{
    /* Stop the Run Loop for this device. */
    CUsbHid* pUsb = (CUsbHid*)context;
    
    printf("usb device removing...\n");
    
    if(pUsb->m_bUsbReady) {
        pUsb->m_bUsbReady = FALSE;
        CFRunLoopStop(pUsb->m_refRunLoop);
    }
    
    if(pUsb->m_pUsbHid_RemovalCallback)
        pUsb->m_pUsbHid_RemovalCallback(pUsb->m_pUsbHid_RemovalParam);
}

void CUsbHid::InputReportCallback(void *context, IOReturn result, void *sender, IOHIDReportType report_type,
                                  uint32_t report_id, uint8_t *report, CFIndex report_length)
{
    CUsbHid* pUsb = (CUsbHid*)context;
    
    pUsb->m_nReceivedLen = sizeof(pUsb->m_recvBuf);
    pUsb->m_nReceivedLen = __min(pUsb->m_nReceivedLen, (int)report_length);
    memcpy(pUsb->m_recvBuf, &report[0], pUsb->m_nReceivedLen);
    
    if(pUsb->m_pUsbHid_RecvCallback)
        pUsb->m_pUsbHid_RecvCallback(pUsb->m_pUsbHid_RecvParam, pUsb->m_recvBuf, pUsb->m_nReceivedLen);
    
    pthread_mutex_lock(&pUsb->m_mutexReceived);
    pthread_cond_signal(&pUsb->m_condReceived);
    pthread_mutex_unlock(&pUsb->m_mutexReceived);
}

void CUsbHid::OutputReportCallback(void *context, IOReturn result, void *sender, IOHIDReportType report_type,
                                   uint32_t report_id, uint8_t *report, CFIndex report_length)
{
    CUsbHid* pUsb = (CUsbHid*)context;
    
    pthread_mutex_lock(&pUsb->m_mutexSended);
    pthread_cond_signal(&pUsb->m_condSended);
    pthread_mutex_unlock(&pUsb->m_mutexSended);
}

void CUsbHid::PerformSignalCallback(void *context)
{
    /* This gets called when the read_thread's run loop gets signaled by
     hid_close(), and serves to stop the read_thread's run loop. */
    CUsbHid* pUsb = (CUsbHid*)context;
    CFRunLoopStop(pUsb->m_refRunLoop);
    
//    printf("PerformSignalCallback\n");
}

void* CUsbHid::RecvThread(void* pParam)
{
    CUsbHid* pUsb = (CUsbHid*)pParam;
    SInt32 code;
    
    /* Move the device's run loop to this thread. */
    IOHIDDeviceScheduleWithRunLoop(pUsb->m_refHidDevice, CFRunLoopGetCurrent(), pUsb->m_refRunLoopMode);
    
    /* Create the RunLoopSource which is used to signal the event loop to stop when hid_close() is called. */
    CFRunLoopSourceContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.version = 0;
    ctx.info = pUsb;
    ctx.perform = &pUsb->PerformSignalCallback;
    pUsb->m_refRunLoopSource = CFRunLoopSourceCreate(kCFAllocatorDefault, 0/*order*/, &ctx);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), pUsb->m_refRunLoopSource, pUsb->m_refRunLoopMode);
    
    /* Store off the Run Loop so it can be stopped from hid_close()
     and on device disconnection. */
    pUsb->m_refRunLoop = CFRunLoopGetCurrent();
    
    /* Notify the main thread that thread is up and running. */
    pUsb->m_bThreadRunning = TRUE;
    pthread_mutex_lock(&pUsb->m_mutexWaitDone);
    pthread_cond_signal(&pUsb->m_condWaitDone);
    pthread_mutex_unlock(&pUsb->m_mutexWaitDone);
    
    /* Run the Event Loop. CFRunLoopRunInMode() will dispatch HID input
     reports into the InputReportCallback(). */
    while (pUsb->m_bUsbReady && pUsb->m_bThreadRunning)
    {
        code = CFRunLoopRunInMode(pUsb->m_refRunLoopMode, 1000/*sec*/, FALSE);
        /* Return if the device has been disconnected */
        if (code == kCFRunLoopRunFinished) {
            pUsb->m_bUsbReady = FALSE;
            break;
        }
        
        /* Break if The Run Loop returns Finished or Stopped. */
        if (code != kCFRunLoopRunTimedOut &&
            code != kCFRunLoopRunHandledSource) {
            break;
        }
    }
    
    /* Now that the read thread is stopping, Wake any threads which are waiting  */
    pthread_mutex_lock(&pUsb->m_mutexWaitExit);
    pthread_cond_broadcast(&pUsb->m_condWaitExit);
    pthread_mutex_unlock(&pUsb->m_mutexWaitExit);
    pUsb->m_bThreadRunning = FALSE;
    
//    printf("thread exit\n");
    return NULL;
}
