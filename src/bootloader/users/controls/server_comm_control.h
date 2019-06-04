#ifndef __SERVER_COMM_CONTROL
#define __SERVER_COMM_CONTROL

#include "typedefs.h"
#include "server_comm_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

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
	
#define COMM_CONTROL_STM32					0x70

#define COMM_CONTROL_IC_READ_REG			0x01
#define COMM_CONTROL_IC_WRITE_REG			0x02
#define COMM_CONTROL_IC_READ_FTP			0x03
#define COMM_CONTROL_IC_WRITE_FTP			0x04
#define COMM_CONTROL_IC_READ_FIFO			0x05
#define COMM_CONTROL_IC_WRITE_FIFO			0x06
#define COMM_CONTROL_STM32_READ_FLASH		0x07
#define COMM_CONTROL_STM32_ERASE_FLASH		0x08
#define COMM_CONTROL_STM32_WRITE_FLASH		0x09

#define COMM_CONTROL_SETU16(x,y)			Comm_SetU16ToBuf(x,y)
#define COMM_CONTROL_GETU16(x)				Comm_GetU16FromBuf(x)
#define COMM_CONTROL_SETU32(x,y)			Comm_SetU32ToBuf(x,y)
#define COMM_CONTROL_GETU32(x)				Comm_GetU32FromBuf(x)

extern Comm_Type g_usbComm;

extern u8 g_nCommCtrlRecvDoneFlag;
extern u16 *g_pCommCtrlRecvLen;
extern u8 *g_pCommCtrlRecvBuf;

void CommControl_Init(void);
u8 CommControl_PhySend(void* pParam, const u8 buf[], u16 len, u32 timeout);
u8 CommControl_SendRetry(u32 timeout);
u8 CommControl_SendBuf(const u8 buf[], u16 len, u32 timeout);
u8 CommControl_RecvBuf(u8 buf[], u16 *pLen, u32 timeout);
u8 CommControl_RecvThread(void);

void CommControl_U16ToU8(const u16 buf16[], u8 buf8[], u16 len8);
void CommControl_U8ToU16(const u8 buf8[], u16 buf16[], u16 len8);
u8 CommControl_Stm32FlashDecode(const u8 recvBuf[], u16 recvLen, u8* nCmd, u8* nSubCmd, u32* addr, u32* len, const u8** pbuf);

u8 CommControl_JustResponseOk(u8 sendBuf[], u16 *pSendLen);
u8 CommControl_Ping(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen);
u8 CommControl_GetFirmware(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen);
u8 CommControl_SoftReset(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen);
u8 CommControl_Stm32Flash(const u8 recvBuf[], u16 recvLen, u8 sendBuf[], u16 *pSendLen);

#ifdef __cplusplus
}
#endif

#endif

