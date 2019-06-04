#include "stdafx.h"
#include "server_comm_protocol.h"
#include <string.h>

/*!
*   @desc   Э���ʼ��
*   @param
*           isMaster �Ƿ�Ϊ���豸��1:���豸 0:���豸
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
*   @desc ���ݷ�������
*/
void Comm_SendReset(Comm_Type *pComm)
{
	memset(pComm->sendBuf, 0, COMM_MAX_PKT_SIZE); 
	pComm->nCurrSendLen = 0;
}

/*!
*   @desc ���ݽ������ã����ݽ�����ɻ��߽��չ����г���ʧ�ܵ���
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
*   @desc ��Ҫ�ṩ�����豸����Э���ʼ����ʱ�򣬿�����һ������ķ��ͼ���
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
*   @desc    ���ݰ����룬���û�����ǰ�������Ϣͷ
*   @param
*            dataBuf  ԭʼ�û�����
*            dataLen  �û����ݳ���
*            sendBuf  ���������ݰ�
*            pSendLen ���������ݳ���
*            nCounter ��ǰ���ݰ����ͼ���
*   @return
*            COMM_ERR_OK     ��ȷ����
*            COMM_ERR_LENGTH �û����ݳ������
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
*   @desc    ���ݰ����룬������ݰ�����
*   @param
*            dataBuf  �����û�����
*            pDataLen �����û����ݳ���
*            recvBuf  ԭʼ�����ݰ�
*            recvLen  ԭʼ�����ݳ���
*            pCounter ���ݰ��еļ���
*   @return
*            COMM_ERR_OK     ��ȷ����
*            COMM_ERR_SYNCID ���ݰ�ͬ��ID����
*            COMM_ERR_LENGTH ���ݰ��������
*            COMM_ERR_CRC    ���ݰ�CRC����
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
*   @desc    �����û����ݣ��Զ�������ݰ�����
*   @param
*            dataBuf  ԭʼ�û�����
*            dataLen  ԭʼ�û����ݳ���
*            timeout  ���ݷ��ͳ�ʱֵ
*            PhySend  ���������ķ��ͺ���
*   @return
*            COMM_ERR_OK           ��ȷ����
*            COMM_ERR_SYNCID       ���ݰ�ͬ��ID����
*            COMM_ERR_LENGTH       ���ݰ��������
*            COMM_ERR_CRC          ���ݰ�CRC����
*            COMM_ERR_TIMEOUT_SEND ����㷢�ͳ�ʱ
*/
u8 Comm_SendBytes(Comm_Type *pComm, const u8 dataBuf[], u16 dataLen, u32 timeout, Comm_pPhySend PhySend, void* pParam)
{
	u8  nRet;

	if(pComm->isMaster)
		pComm->nSendCounter++;	// ���豸���ܶԷ��ͼ�������

	nRet = Comm_Encoder(pComm, dataBuf, dataLen, pComm->sendBuf, &pComm->nCurrSendLen, pComm->nSendCounter);
	if(COMM_ERR_OK != nRet)
		return nRet;

	nRet = PhySend(pParam, pComm->sendBuf, pComm->nCurrSendLen, timeout);

	return nRet;
}

/*!
*   @desc    �����û����ݣ�ÿһ������һ��Byte�����ջ��棬�ڲ��Զ������ݽ��м�飬���ݷ���ֵ�жϽ����Ƿ����
*   @param
*            nByte    ���յ�Byte
*            dataBuf  �����û�����
*            dataLen  �����û����ݳ���
*   @return
*            COMM_ERR_OK          ��ȷ����
*            COMM_ERR_RECEIVING   ���ڽ��գ���ʾ����δ���
*            COMM_ERR_RECV        ���մ���
*            COMM_ERR_SYNCID      ͬ��ID����
*            COMM_ERR_LENGTH      ���ݰ��������
*            COMM_ERR_CRC         ���ݰ�CRC����
*            COMM_ERR_COUNTER     ���ռ�������
*            COMM_ERR_RECV_REPEAT ���յ��ظ��İ� 
*/
u8 Comm_RecvBytes(Comm_Type *pComm, u8 nByte, u8 dataBuf[], u16 *pDataLen)
{ 
	u8 nRet = COMM_ERR_RECEIVING;
	pComm->recvBuf[pComm->nCurrRecvLen++] = nByte;

	if(0==pComm->isSynIdPass && pComm->nCurrRecvLen==COMM_INDEX_SYNCID+1)
	{
		if(COMM_SYNC_ID==COMM_GET_SYNCID(pComm->recvBuf))
		{
			// ͬ��ID���ͨ��
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
			// ���ݳ��ȺϷ�
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
			// ��ͷCRC����ʧ�ܣ����ý���
			Comm_RecvReset(pComm);
		}
	}

	else if(pComm->nCurrRecvLen >= pComm->nNeedRecvLen 
		&& pComm->nCurrRecvLen > COMM_HEAD_INFO_SIZE)
	{
		u32 nCounter;
		u16 nCrc;

		// ���ݰ�������ɣ�������ݰ���ȡ���û�����
		nRet = Comm_Decoder(pComm, dataBuf, pDataLen, pComm->recvBuf, pComm->nCurrRecvLen, &nCounter);

		if(COMM_ERR_OK==nRet)
		{
			nCrc = Comm_Crc16Calc(pComm, pComm->recvBuf, pComm->nCurrRecvLen);

			if(pComm->isMaster)
			{
				if(pComm->nSendCounter != nCounter)
					nRet = COMM_ERR_COUNTER;	// ���ռ�������û���յ���ȷ�ļ������������豸ÿ��ͨѶ�ļ���ֵ����һ�������豸��Ӧʱ����ı䵱ǰ������
			}
			else
			{
				pComm->nSendCounter = nCounter;
				if(pComm->nRecvCounter==nCounter && pComm->nRecvCrc==nCrc)
					nRet = COMM_ERR_RECV_REPEAT;	// ���ռ����ظ�

			}

			// �����������ݰ���CRC�Ͱ������������´ν��շ���
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
*   @desc    �ط�ǰһ�ε����ݰ������������豸���ճ�ʱ�ش���
*            Ҳ�����ڴ��豸���յ��ش���ʱ������һ�ε�Ӧ����һ�ε�Ӧ��ʧ�����豸�Ż��ش���
*   @param
*            timeout  ���ͳ�ʱֵ
*            PhySend  ����㷢�ͻص�����
*   @return
*            COMM_ERR_OK           ��ȷ����
*            COMM_ERR_SYNCID       �ش���ͬ��ID����
*            COMM_ERR_LENGTH       �ش������������
*            COMM_ERR_CRC          �ش�����CRC����
*            COMM_ERR_TIMEOUT_SEND ����㷢�ͳ�ʱ
*/
u8 Comm_SendRetry(Comm_Type *pComm, u32 timeout, Comm_pPhySend PhySend, void* pParam)
{
	static u8 tmpBuf[COMM_MAX_DATA_SIZE];
	u8 nRet;
	u16 tmpLen;
	u32 tmpCounter;

	// ������ݰ��Ƿ���Ч
	nRet = Comm_Decoder(pComm, tmpBuf, &tmpLen, pComm->sendBuf, pComm->nCurrSendLen, &tmpCounter);

	if(COMM_ERR_OK==nRet)
		nRet = PhySend(pParam, pComm->sendBuf, pComm->nCurrSendLen, timeout);

	return nRet;
}
