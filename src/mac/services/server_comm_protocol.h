/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, CMOSTEK SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * Copyright (C) CMOSTEK SZ.
 */

/*!
 * @file        server_comm_protocol.h
 * @brief       communication protocol for Offline-Writer
 *
 * @version     1.0
 * @date        Oct 27 2015
 * @author      CMOSTEK R@D
 */

#ifndef __SERVER_COMM_PROTOCOL
#define __SERVER_COMM_PROTOCOL

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMM_CRC_POLY			0x1021      // CRC生成多项式(CCITT)
#define COMM_SYNC_ID			0x68        // 包头同步ID
#define COMM_HEAD_INFO_SIZE	11          // 包头长度
#define COMM_MAX_DATA_SIZE		512         // 用户数据最大长度
#define COMM_MAX_PKT_SIZE		(COMM_HEAD_INFO_SIZE + COMM_MAX_DATA_SIZE)  // 数据包最大长度
	

#define COMM_ERR_OK				0x00   // 返回正确
#define COMM_ERR_RECEIVING		0x01   // 正在接收
#define COMM_ERR_SYNCID			0x02   // 包同步ID错误
#define COMM_ERR_LENGTH			0x03   // 包长度错误
#define COMM_ERR_COUNTER		0x04   // 包计数错误
#define COMM_ERR_CRC			0x05   // CRC错误
#define COMM_ERR_RECV_REPEAT	0x06   // 接收到重复的数据包
#define COMM_ERR_TIMEOUT_SEND	0x07   // 发送超时
#define COMM_ERR_TIMEOUT_RECV	0x08   // 接收超时
#define COMM_ERR_SEND			0x81   // 发送错误
#define COMM_ERR_RECV			0x82   // 接收错误


#define COMM_INDEX_SYNCID		0   // 包同步ID位置
#define COMM_INDEX_LENGTH		1   // 包长度位置
#define COMM_INDEX_COUNTER		3   // 包计数位置
#define COMM_INDEX_DATACRC		7   // 用户数据CRC位置
#define COMM_INDEX_HEADCRC		9   // 包头CRC位置

#define COMM_GET_SYNCID(buf)	(buf[COMM_INDEX_SYNCID])
#define COMM_GET_LENGTH(buf)	Comm_GetU16FromBuf(&buf[COMM_INDEX_LENGTH])
#define COMM_GET_COUNTER(buf)	Comm_GetU32FromBuf(&buf[COMM_INDEX_COUNTER])
#define COMM_GET_DATACRC(buf)	Comm_GetU16FromBuf(&buf[COMM_INDEX_DATACRC])
#define COMM_GET_HEADCRC(buf)	Comm_GetU16FromBuf(&buf[COMM_INDEX_HEADCRC])

#define COMM_SET_SYNCID(buf,x)	(buf[COMM_INDEX_SYNCID] = x)
#define COMM_SET_LENGTH(buf,x)	Comm_SetU16ToBuf(&buf[COMM_INDEX_LENGTH], x)
#define COMM_SET_COUNTER(buf,x)	Comm_SetU32ToBuf(&buf[COMM_INDEX_COUNTER],x)
#define COMM_SET_DATACRC(buf,x)	Comm_SetU16ToBuf(&buf[COMM_INDEX_DATACRC],x)
#define COMM_SET_HEADCRC(buf,x)	Comm_SetU16ToBuf(&buf[COMM_INDEX_HEADCRC],x)


/* 包头信息 */
typedef struct __Comm_HeadInfo{
	u8  nSyncId;    // 同步ID
	u16 nLength;    // 用户数据长度
	u32 nCounter;   // 数据包计数器
	u16 nDataCrc;   // 用户数据CRC
	u16 nHeadCrc;   // 包头CRC
}Comm_HeadInfo;

typedef struct __Comm_Type {
	u8  isMaster;			// 是否为主设备
	u16 nNeedRecvLen;		// 需要接收的包长度
	u16 nCurrRecvLen;		// 当前接收的数据长度
	u16 nCurrSendLen;		// 当前发送的数据长度
	u8  isSynIdPass;		// 同步ID验证通过
	u8  isLengthPass;		// 长度信息验证通过

	u32 nRecvCounter;	    // 数据接收计数器
	u32 nSendCounter;	    // 数据发送计数器
	u16 nRecvCrc;	        // 上次接收到数据CRC，用于判断接收的数据是否重复

	u16 crcTable[256];		// CRC检验查表
	u8  sendBuf[COMM_MAX_PKT_SIZE];   // 数据发送缓存
	u8  recvBuf[COMM_MAX_PKT_SIZE];   // 数据接收缓存
}Comm_Type;

typedef u8 (*Comm_pPhySend)(void* pParam, const u8 sendBuf[], u16 sendLen, u32 timeout);

void Comm_Init(Comm_Type *pComm, u8 isMaster);
void Comm_Crc16Init(Comm_Type *pComm);
u16 Comm_Crc16Calc(Comm_Type *pComm, const u8 buf[], u16 len);

void Comm_SetU16ToBuf(u8 buf[], u16 dat16);
u16 Comm_GetU16FromBuf(const u8 buf[]);

void Comm_SetU32ToBuf(u8 buf[], u32 dat32);
u32 Comm_GetU32FromBuf(const u8 buf[]);

void Comm_SendReset(Comm_Type *pComm);
void Comm_RecvReset(Comm_Type *pComm);
void Comm_SetSendCounter(Comm_Type *pComm, u32 nCounter);
void Comm_GetHeadFromBuf(Comm_HeadInfo* pHeadInfo, const u8 buf[]);
void Comm_SetHeadToBuf(const Comm_HeadInfo* pHeadInfo, u8 buf[]);
u8 Comm_Encoder(Comm_Type *pComm, const u8 dataBuf[], u16 dataLen, u8 sendBuf[], u16 *pSendLen, u32 nCounter); 
u8 Comm_Decoder(Comm_Type *pComm, u8 dataBuf[], u16 *pDataLen, const u8 recvBuf[], u16 recvLen, u32 *pCounter);

u8 Comm_SendBytes(Comm_Type *pComm, const u8 dataBuf[], u16 dataLen, u32 timeout, Comm_pPhySend PhySend, void* pParam);
u8 Comm_RecvBytes(Comm_Type *pComm, u8 nByte, u8 dataBuf[], u16 *pDataLen);

u8 Comm_SendRetry(Comm_Type *pComm, u32 timeout, Comm_pPhySend PhySend, void* pParam);

#ifdef __cplusplus
}
#endif

#endif

