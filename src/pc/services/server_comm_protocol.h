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

#define COMM_CRC_POLY			0x1021      // CRC���ɶ���ʽ(CCITT)
#define COMM_SYNC_ID			0x68        // ��ͷͬ��ID
#define COMM_HEAD_INFO_SIZE	11          // ��ͷ����
#define COMM_MAX_DATA_SIZE		512         // �û�������󳤶�
#define COMM_MAX_PKT_SIZE		(COMM_HEAD_INFO_SIZE + COMM_MAX_DATA_SIZE)  // ���ݰ���󳤶�
	

#define COMM_ERR_OK				0x00   // ������ȷ
#define COMM_ERR_RECEIVING		0x01   // ���ڽ���
#define COMM_ERR_SYNCID			0x02   // ��ͬ��ID����
#define COMM_ERR_LENGTH			0x03   // �����ȴ���
#define COMM_ERR_COUNTER		0x04   // ����������
#define COMM_ERR_CRC			0x05   // CRC����
#define COMM_ERR_RECV_REPEAT	0x06   // ���յ��ظ������ݰ�
#define COMM_ERR_TIMEOUT_SEND	0x07   // ���ͳ�ʱ
#define COMM_ERR_TIMEOUT_RECV	0x08   // ���ճ�ʱ
#define COMM_ERR_SEND			0x81   // ���ʹ���
#define COMM_ERR_RECV			0x82   // ���մ���


#define COMM_INDEX_SYNCID		0   // ��ͬ��IDλ��
#define COMM_INDEX_LENGTH		1   // ������λ��
#define COMM_INDEX_COUNTER		3   // ������λ��
#define COMM_INDEX_DATACRC		7   // �û�����CRCλ��
#define COMM_INDEX_HEADCRC		9   // ��ͷCRCλ��

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


/* ��ͷ��Ϣ */
typedef struct __Comm_HeadInfo{
	u8  nSyncId;    // ͬ��ID
	u16 nLength;    // �û����ݳ���
	u32 nCounter;   // ���ݰ�������
	u16 nDataCrc;   // �û�����CRC
	u16 nHeadCrc;   // ��ͷCRC
}Comm_HeadInfo;

typedef struct __Comm_Type {
	u8  isMaster;			// �Ƿ�Ϊ���豸
	u16 nNeedRecvLen;		// ��Ҫ���յİ�����
	u16 nCurrRecvLen;		// ��ǰ���յ����ݳ���
	u16 nCurrSendLen;		// ��ǰ���͵����ݳ���
	u8  isSynIdPass;		// ͬ��ID��֤ͨ��
	u8  isLengthPass;		// ������Ϣ��֤ͨ��

	u32 nRecvCounter;	    // ���ݽ��ռ�����
	u32 nSendCounter;	    // ���ݷ��ͼ�����
	u16 nRecvCrc;	        // �ϴν��յ�����CRC�������жϽ��յ������Ƿ��ظ�

	u16 crcTable[256];		// CRC������
	u8  sendBuf[COMM_MAX_PKT_SIZE];   // ���ݷ��ͻ���
	u8  recvBuf[COMM_MAX_PKT_SIZE];   // ���ݽ��ջ���
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

