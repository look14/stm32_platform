#ifndef __SERVER_COMM_CONTROL
#define __SERVER_COMM_CONTROL

#include "typedefs.h"
#include "UsbHid.h"

#include "server_comm_protocol.h"

#define COMM_CONTROL_ERR_OK					0x00
#define COMM_CONTROL_ERR_COMM				0x01
#define COMM_CONTROL_ERR_DATA_INFO			0x02
#define COMM_CONTROL_ERR_READ_REG			0x03
#define COMM_CONTROL_ERR_WRITE_REG			0x04
#define COMM_CONTROL_ERR_READ_FTP			0x05
#define COMM_CONTROL_ERR_WRITE_FTP			0x06
#define COMM_CONTROL_ERR_READ_FLASH			0x07
#define COMM_CONTROL_ERR_ERASE_FLASH		0x08
#define COMM_CONTROL_ERR_WRITE_FLASH		0x09
#define COMM_CONTROL_ERR_OTHER				0x80

#define COMM_CONTROL_PING					0x10
#define COMM_CONTROL_FIRMWARE				0x11
#define COMM_CONTROL_SOFT_RESET				0x12
#define COMM_CONTROL_ENTER_USB				0x13
#define COMM_CONTROL_LEAVE_USB				0x14
#define COMM_CONTROL_GPIO_OPERA          	0x17
#define COMM_CONTROL_RTC                    0x18
	
#define COMM_CONTROL_STM32					0x70
#define COMM_CONTROL_MPU6050            	0x71

#define COMM_CONTROL_IC_READ_REG			0x01
#define COMM_CONTROL_IC_WRITE_REG			0x02
#define COMM_CONTROL_IC_READ_FTP			0x03
#define COMM_CONTROL_IC_WRITE_FTP			0x04
#define COMM_CONTROL_IC_READ_FIFO			0x05
#define COMM_CONTROL_IC_WRITE_FIFO			0x06
#define COMM_CONTROL_STM32_READ_FLASH		0x07
#define COMM_CONTROL_STM32_ERASE_FLASH		0x08
#define COMM_CONTROL_STM32_WRITE_FLASH		0x09

#define COMM_CONTROL_GPIO_OUT            	0x71
#define COMM_CONTROL_GPIO_IN             	0x72
#define COMM_CONTROL_GPIO_HIGH           	0x73
#define COMM_CONTROL_GPIO_LOW            	0x74
#define COMM_CONTROL_GPIO_READ           	0x75

#define COMM_CONTROL_RTC_UPDATE_TIME        0x01
#define COMM_CONTROL_RTC_GET_TIME           0x02

#define COMM_CONTROL_SETU16(x,y)			Comm_SetU16ToBuf(x,y)
#define COMM_CONTROL_GETU16(x)				Comm_GetU16FromBuf(x)
#define COMM_CONTROL_SETU32(x,y)			Comm_SetU32ToBuf(x,y)
#define COMM_CONTROL_GETU32(x)				Comm_GetU32FromBuf(x)

class CommControl
{
protected:
	CommControl();

public:
	virtual ~CommControl();

	static CommControl* GetInstance();

	BOOL ReopenUsb();
	void CloseUsb();
	BOOL IsUsbReady()		{ return m_usbHid.Ready(); }
	char* GetUsbVendor()	{ return m_usbHid.m_strVendor; }
	char* GetUsbProduct()	{ return m_usbHid.m_strProduct; }

	void Init(void);
	static u8 PhySend(void* pParam, const u8 buf[], u16 len, u32 timeout);
	u8 SendRetry(u32 timeout);
	u8 SendBuf(const u8 buf[], u16 len, u32 timeout);
	u8 RecvBuf(u8 buf[], u16 *pLen, u32 timeout);
	u8 Transfer(u8 sendBuf[], u16 sendLen, u8 recvBuf[], u16 *recvLen, u32 timeout);
	
    static void UsbRemovalCallback(void* pParam);
	static u8 RecvCallback(void* pParam, const u8 recvBuf[], u16 recvLen);
	static unsigned int RecvThread(LPVOID lpVoid);
	
	static void U16ToU8(const u16 buf16[], u8 buf8[], u16 len8);
	static void U8ToU16(const u8 buf8[], u16 buf16[], u16 len8);
	
	u8 SendCmd(u8 nCmd);
	u8 IcReadBuf(u8 nCmd, u8 nSubCmd, u8 addr, u8 buf[], u16 len);
	u8 IcWriteBuf(u8 nCmd, u8 nSubCmd, u8 addr, const u8 buf[], u16 len);
	
	u8 Ping();
	u8 GetFirmware(u8 buf[], u16 *plen);
	u8 SoftReset();
	u8 EnterUsb();
	u8 LeaveUsb();

	u8 Stm32ReadFlash(u32 addr, u8 buf[], u32 len);
	u8 Stm32EraseFlash(u32 addr, u32 len);
	u8 Stm32WriteFlash(u32 addr, const u8 buf[], u32 len);

	u8 SetGpioOut(u32 nGpioType, u16 nGpioPin);
	u8 SetGpioIn(u32 nGpioType, u16 nGpioPin);
	u8 SetGpioHigh(u32 nGpioType, u16 nGpioPin);
	u8 SetGpioLow(u32 nGpioType, u16 nGpioPin);
	u8 ReadGpio(u32 nGpioType, u16 nGpioPin, u8 &nRes);
    
#if 0
	u8 RtcUpdateTime(const SYSTEMTIME* ptm);
	u8 RtcGetTime(SYSTEMTIME* ptm);
#endif

	u8 Mpu6050ReadRegs(u8 addr, u8 buf[], u16 len);
	u8 Mpu6050WriteRegs(u8 addr, const u8 buf[], u16 len);

protected:
	CUsbHid m_usbHid;
	Comm_Type m_usbComm;

	u8 m_nCommCtrlRecvDoneFlag;
	u16* m_pCommCtrlRecvLen;
	u8*  m_pCommCtrlRecvBuf;
	
    u16 m_nCommCtrlSendLen;
    u16 m_nCommCtrlRecvLen;
    u8  m_strCommCtrlSendBuf[COMM_MAX_DATA_SIZE];
    u8  m_strCommCtrlRecvBuf[COMM_MAX_DATA_SIZE];
    u8  m_strCommCtrlTmpBuf8[COMM_MAX_DATA_SIZE];
    u16 m_strCommCtrlTmpBuf16[COMM_MAX_DATA_SIZE/2];
	
public:
	u16	m_nUsbVendorId;
	u16 m_nUsbProductId;
};

#endif

