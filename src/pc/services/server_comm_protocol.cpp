#include "stdafx.h"
#include "server_comm_protocol.h"
#include <string.h>

/*!
*   @desc   协议初始化
*   @param
*           isMaster 是否为主设备，1:主设备 0:从设备
*/
void Comm_Init(Comm_Type *pComm, u8 isMaster)
{
	Comm_Crc16Init(pComm);
	Comm_SendReset(pComm);
	Comm_RecvReset(pComm);
	pComm->isMaster     = isMaster;
	pComm->nSendCounter = 0; 
	pComm->nRecvCounter = 0;
	pComm->nRecvCrc     = 0;
}

void Comm_Crc16Init(Comm_Type *pComm)
{
	u16 remainder;  
	u16 dividend;  
	u8 i;  
	/* Perform binary long division, a bit at a time. */  
	for(dividend = 0; dividend < 256; dividend++)  
	{  
		/* Initialize the remainder.  */  
		remainder = dividend << 8;
		/* Shift and XOR with the polynomial.   */  
		for(i = 0; i < 8; i++)  
		{  
			/* Try to divide the current data bit.  */  
			if(remainder & 0x8000)  
			{  
				remainder = (remainder << 1) ^ COMM_CRC_POLY;  
			}  
			else  
			{  
				remainder = remainder << 1;  
			}  
		}  
		/* Save the result in the table. */  
		pComm->crcTable[dividend] = remainder;  
	}
}

u16 Comm_Crc16Calc(Comm_Type *pComm, const u8 buf[], u16 len)
{
	u16 crc = 0x0000;
	u16 offset;  
	u8 byte;

	/* Divide the message by the polynomial, a byte at a time. */  
	for( offset = 0; offset < len; offset++)  
	{  
		byte = (crc >> 8) ^ buf[offset];  
		crc = pComm->crcTable[byte] ^ (crc << 8);  
	}  
	/* The final remainder is the CRC result. */  
	return (crc ^ 0xFFFF);  
}

/*!
*   @desc 数据发送重置
*/
void Comm_SendReset(Comm_Type *pComm)
{
	memset(pComm->sendBuf, 0, COMM_MAX_PKT_SIZE); 
	pComm->nCurrSendLen = 0;
}

/*!
*   @desc 数据接收重置，数据接收完成或者接收过程中出现失败调用
*/
void Comm_RecvReset(Comm_Type *pComm)
{ 
	pComm->nNeedRecvLen = 0;
	pComm->nCurrRecvLen = 0;
	pComm->isSynIdPass  = 0;
	pComm->isLengthPass = 0; 
	memset(pComm->recvBuf, 0, COMM_MAX_PKT_SIZE);
}

/*!
*   @desc 主要提供给主设备，对协议初始化的时候，可设置一个随机的发送计数
*/
void Comm_SetSendCounter(Comm_Type *pComm, u32 nCounter)
{
	pComm->nSendCounter = nCounter;
}

void Comm_SetU16ToBuf(u8 buf[], u16 dat16)
{
	buf[0] = (u8)dat16;
	buf[1] = (u8)(dat16 >> 8);
}

u16 Comm_GetU16FromBuf(const u8 buf[])
{
	u16 dat16 = 0;
	dat16  = buf[0];
	dat16 |= ((u16)buf[1]) << 8;
	return dat16;
}

void Comm_SetU32ToBuf(u8 buf[], u32 dat32)
{
	buf[0] = (u8)dat32;
	buf[1] = (u8)(dat32 >> 8);
	buf[2] = (u8)(dat32 >> 16);
	buf[3] = (u8)(dat32 >> 24);
}

u32 Comm_GetU32FromBuf(const u8 buf[])
{
	u32 dat32 = 0;
	dat32  = buf[0];
	dat32 |= ((u32)buf[1]) << 8;
	dat32 |= ((u32)buf[2]) << 16;
	dat32 |= ((u32)buf[3]) << 24;
	return dat32;
}

void Comm_GetHeadFromBuf(Comm_HeadInfo* pHeadInfo, const u8 buf[])
{
	pHeadInfo->nSyncId  = COMM_GET_SYNCID(buf);
	pHeadInfo->nLength  = COMM_GET_LENGTH(buf);
	pHeadInfo->nCounter = COMM_GET_COUNTER(buf);
	pHeadInfo->nDataCrc = COMM_GET_DATACRC(buf);
	pHeadInfo->nHeadCrc = COMM_GET_HEADCRC(buf);
}

void Comm_SetHeadToBuf(const Comm_HeadInfo* pHeadInfo, u8 buf[])
{
	COMM_SET_SYNCID(buf, pHeadInfo->nSyncId);
	COMM_SET_LENGTH(buf, pHeadInfo->nLength);
	COMM_SET_COUNTER(buf, pHeadInfo->nCounter);
	COMM_SET_DATACRC(buf, pHeadInfo->nDataCrc);
	COMM_SET_HEADCRC(buf, pHeadInfo->nHeadCrc);
}

/*!
*   @desc    数据包编码，在用户数据前面加入信息头
*   @param
*            dataBuf  原始用户数据
*            dataLen  用户数据长度
*            sendBuf  编码后的数据包
*            pSendLen 编码后的数据长度
*            nCounter 当前数据包发送计数
*   @return
*            COMM_ERR_OK     正确返回
*            COMM_ERR_LENGTH 用户数据长度溢出
*/
u8 Comm_Encoder(Comm_Type *pComm, const u8 dataBuf[], u16 dataLen, u8 sendBuf[], u16 *pSendLen, u32 nCounter)
{
	static Comm_HeadInfo headInfo;

	if(dataLen > COMM_MAX_DATA_SIZE || dataLen==0)
		return COMM_ERR_LENGTH;

	headInfo.nSyncId  = COMM_SYNC_ID;
	headInfo.nLength  = dataLen;
	headInfo.nCounter = nCounter;
	headInfo.nDataCrc = Comm_Crc16Calc(pComm, dataBuf, dataLen);

	Comm_SetHeadToBuf(&headInfo, sendBuf);

	headInfo.nHeadCrc = Comm_Crc16Calc(pComm, sendBuf, COMM_HEAD_INFO_SIZE-2);
	COMM_SET_HEADCRC(sendBuf, headInfo.nHeadCrc);

	memcpy(&sendBuf[COMM_HEAD_INFO_SIZE], dataBuf, dataLen);
	*pSendLen = COMM_HEAD_INFO_SIZE + dataLen;

	return COMM_ERR_OK;
}

/*!
*   @desc    数据包解码，检查数据包错误
*   @param
*            dataBuf  返回用户数据
*            pDataLen 返回用户数据长度
*            recvBuf  原始的数据包
*            recvLen  原始的数据长度
*            pCounter 数据包中的计数
*   @return
*            COMM_ERR_OK     正确返回
*            COMM_ERR_SYNCID 数据包同步ID错误
*            COMM_ERR_LENGTH 数据包长度溢出
*            COMM_ERR_CRC    数据包CRC错误
*/
u8 Comm_Decoder(Comm_Type *pComm, u8 dataBuf[], u16 *pDataLen, const u8 recvBuf[], u16 recvLen, u32 *pCounter)
{
	static Comm_HeadInfo headInfo;

	if(recvLen > COMM_MAX_PKT_SIZE)
		return COMM_ERR_LENGTH;

	Comm_GetHeadFromBuf(&headInfo, recvBuf);

	if(headInfo.nSyncId != COMM_SYNC_ID)
		return COMM_ERR_SYNCID;

	if(headInfo.nLength > COMM_MAX_DATA_SIZE || headInfo.nLength==0)
		return COMM_ERR_LENGTH;

	if(headInfo.nHeadCrc != Comm_Crc16Calc(pComm, recvBuf, COMM_INDEX_HEADCRC))
		return COMM_ERR_CRC;

	if(headInfo.nDataCrc != Comm_Crc16Calc(pComm, &recvBuf[COMM_HEAD_INFO_SIZE], headInfo.nLength))
		return COMM_ERR_CRC;

	*pDataLen = headInfo.nLength;
	*pCounter = headInfo.nCounter;
	memcpy(dataBuf, &recvBuf[COMM_HEAD_INFO_SIZE], headInfo.nLength);

	return COMM_ERR_OK;
}

/*!
*   @desc    发送用户数据，自动完成数据包编码
*   @param
*            dataBuf  原始用户数据
*            dataLen  原始用户数据长度
*            timeout  数据发送超时值
*            PhySend  调用物理层的发送函数
*   @return
*            COMM_ERR_OK           正确返回
*            COMM_ERR_SYNCID       数据包同步ID错误
*            COMM_ERR_LENGTH       数据包长度溢出
*            COMM_ERR_CRC          数据包CRC错误
*            COMM_ERR_TIMEOUT_SEND 物理层发送超时
*/
u8 Comm_SendBytes(Comm_Type *pComm, const u8 dataBuf[], u16 dataLen, u32 timeout, Comm_pPhySend PhySend, void* pParam)
{
	u8  nRet;

	if(pComm->isMaster)
		pComm->nSendCounter++;	// 主设备才能对发送计数递增

	nRet = Comm_Encoder(pComm, dataBuf, dataLen, pComm->sendBuf, &pComm->nCurrSendLen, pComm->nSendCounter);
	if(COMM_ERR_OK != nRet)
		return nRet;

	nRet = PhySend(pParam, pComm->sendBuf, pComm->nCurrSendLen, timeout);

	return nRet;
}

/*!
*   @desc    接收用户数据，每一次增加一个Byte到接收缓存，内部自动对数据进行检查，根据返回值判断接收是否完成
*   @param
*            nByte    接收到Byte
*            dataBuf  返回用户数据
*            dataLen  返回用户数据长度
*   @return
*            COMM_ERR_OK          正确接收
*            COMM_ERR_RECEIVING   正在接收，表示接收未完成
*            COMM_ERR_RECV        接收错误
*            COMM_ERR_SYNCID      同步ID错误
*            COMM_ERR_LENGTH      数据包长度溢出
*            COMM_ERR_CRC         数据包CRC错误
*            COMM_ERR_COUNTER     接收计数错误
*            COMM_ERR_RECV_REPEAT 接收到重复的包 
*/
u8 Comm_RecvBytes(Comm_Type *pComm, u8 nByte, u8 dataBuf[], u16 *pDataLen)
{ 
	u8 nRet = COMM_ERR_RECEIVING;
	pComm->recvBuf[pComm->nCurrRecvLen++] = nByte;

	if(0==pComm->isSynIdPass && pComm->nCurrRecvLen==COMM_INDEX_SYNCID+1)
	{
		if(COMM_SYNC_ID==COMM_GET_SYNCID(pComm->recvBuf))
		{
			// 同步ID检查通过
			pComm->isSynIdPass = 1;
		}
		else
			Comm_RecvReset(pComm);
	}

	else if(0==pComm->isLengthPass && pComm->nCurrRecvLen==COMM_INDEX_LENGTH+2)
	{
		pComm->nNeedRecvLen = COMM_GET_LENGTH(pComm->recvBuf);
		if(pComm->nNeedRecvLen > 0 && pComm->nNeedRecvLen <= COMM_MAX_DATA_SIZE)
		{
			// 数据长度合法
			pComm->nNeedRecvLen += COMM_HEAD_INFO_SIZE;
			pComm->isLengthPass  = 1; 
		}
		else
			Comm_RecvReset(pComm);
	}

	else if(pComm->nCurrRecvLen==COMM_INDEX_HEADCRC+2)
	{
		if(COMM_GET_HEADCRC(pComm->recvBuf) != 
			Comm_Crc16Calc(pComm, pComm->recvBuf, COMM_INDEX_HEADCRC))
		{
			// 包头CRC检验失败，重置接收
			Comm_RecvReset(pComm);
		}
	}

	else if(pComm->nCurrRecvLen >= pComm->nNeedRecvLen 
		&& pComm->nCurrRecvLen > COMM_HEAD_INFO_SIZE)
	{
		u32 nCounter;
		u16 nCrc;

		// 数据包接收完成，检查数据包并取出用户数据
		nRet = Comm_Decoder(pComm, dataBuf, pDataLen, pComm->recvBuf, pComm->nCurrRecvLen, &nCounter);

		if(COMM_ERR_OK==nRet)
		{
			nCrc = Comm_Crc16Calc(pComm, pComm->recvBuf, pComm->nCurrRecvLen);

			if(pComm->isMaster)
			{
				if(pComm->nSendCounter != nCounter)
					nRet = COMM_ERR_COUNTER;	// 接收计数错误，没有收到正确的计数（对于主设备每次通讯的计数值都不一样，从设备响应时不会改变当前计数）
			}
			else
			{
				pComm->nSendCounter = nCounter;
				if(pComm->nRecvCounter==nCounter && pComm->nRecvCrc==nCrc)
					nRet = COMM_ERR_RECV_REPEAT;	// 接收计数重复

			}

			// 保存整个数据包的CRC和包计数，用于下次接收分析
			pComm->nRecvCrc     = nCrc;
			pComm->nRecvCounter = nCounter;
		}

		Comm_RecvReset(pComm);
	}

	if(pComm->nCurrRecvLen >= COMM_MAX_PKT_SIZE)
		Comm_RecvReset(pComm);

	return nRet;
}

/*!
*   @desc    重发前一次的数据包，可用于主设备接收超时重传，
*            也可用于从设备接收到重传包时接续上一次的应答（上一次的应答丢失，主设备才会重传）
*   @param
*            timeout  发送超时值
*            PhySend  物理层发送回调函数
*   @return
*            COMM_ERR_OK           正确发送
*            COMM_ERR_SYNCID       重传包同步ID错误
*            COMM_ERR_LENGTH       重传包包长度溢出
*            COMM_ERR_CRC          重传包包CRC错误
*            COMM_ERR_TIMEOUT_SEND 物理层发送超时
*/
u8 Comm_SendRetry(Comm_Type *pComm, u32 timeout, Comm_pPhySend PhySend, void* pParam)
{
	static u8 tmpBuf[COMM_MAX_DATA_SIZE];
	u8 nRet;
	u16 tmpLen;
	u32 tmpCounter;

	// 检查数据包是否有效
	nRet = Comm_Decoder(pComm, tmpBuf, &tmpLen, pComm->sendBuf, pComm->nCurrSendLen, &tmpCounter);

	if(COMM_ERR_OK==nRet)
		nRet = PhySend(pParam, pComm->sendBuf, pComm->nCurrSendLen, timeout);

	return nRet;
}
