#include "stdafx.h"
#include "server_config.h"
#include <string.h>

u16 gServerConfig_crcTable[256];
u32 gServerConfig_nBaseAddr;

pServerConfig_ReadFlash ServerConfig_ReadFlash;
pServerConfig_WriteFlash ServerConfig_WriteFlash;

u8 ServerConfig_Init(u32 nBaseAddr, pServerConfig_ReadFlash pReadFlash, pServerConfig_WriteFlash pWriteFlash)
{
	ServerConfig_Crc16Init();

	gServerConfig_nBaseAddr = nBaseAddr;
	ServerConfig_ReadFlash = pReadFlash;
	ServerConfig_WriteFlash = pWriteFlash;

	return SERVER_CONFIG_ERR_OK;
}

u8 ServerConfig_DecodeNode(ServerConfig_Node* pNode, const u8 buf[])
{
	u8 nRet = SERVER_CONFIG_ERR_OK;
	u8 nIndex = 0;

	pNode->nId			= ServerConfig_GetU32FromBuf(&buf[nIndex]);	nIndex += 4;
	pNode->nSize		= ServerConfig_GetU32FromBuf(&buf[nIndex]);	nIndex += 4;
	pNode->nLength		= ServerConfig_GetU32FromBuf(&buf[nIndex]);	nIndex += 4;
	pNode->nNextPos		= ServerConfig_GetU32FromBuf(&buf[nIndex]);	nIndex += 4;
	pNode->nContentCrc	= ServerConfig_GetU16FromBuf(&buf[nIndex]);	nIndex += 2;
	pNode->nNodeCrc		= ServerConfig_GetU16FromBuf(&buf[nIndex]);	nIndex += 2;

	if(SERVER_CONFIG_ERR_OK==nRet) {
		u32 i = 0;

		for(i=0; i<SERVER_CONFIG_NODE_LEN; i++) {
			if(buf[i] != 0xFF)
				break;
		}

		if(i >= SERVER_CONFIG_NODE_LEN)
			nRet = SERVER_CONFIG_ERR_NULL;
	}

	if(SERVER_CONFIG_ERR_OK==nRet && ServerConfig_Crc16Calc(buf, SERVER_CONFIG_NODE_LEN-2) != pNode->nNodeCrc)
		nRet = SERVER_CONFIG_ERR_NODE_CRC;

	if(SERVER_CONFIG_ERR_OK==nRet && pNode->nLength > pNode->nSize)
		nRet = SERVER_CONFIG_ERR_LENGTH;

	return nRet;
}

u8 ServerConfig_EncodeNode(const ServerConfig_Node* pNode, u8 buf[])
{
	u8 nRet = SERVER_CONFIG_ERR_OK;
	u8 nIndex = 0;
	
	ServerConfig_SetU32ToBuf(&buf[nIndex], pNode->nId);			nIndex += 4;
	ServerConfig_SetU32ToBuf(&buf[nIndex], pNode->nSize);		nIndex += 4;
	ServerConfig_SetU32ToBuf(&buf[nIndex], pNode->nLength);		nIndex += 4;
	ServerConfig_SetU32ToBuf(&buf[nIndex], pNode->nNextPos);	nIndex += 4;
	ServerConfig_SetU16ToBuf(&buf[nIndex], pNode->nContentCrc);	nIndex += 2;

	ServerConfig_SetU16ToBuf(&buf[nIndex], ServerConfig_Crc16Calc(buf, SERVER_CONFIG_NODE_LEN-2));

	if(SERVER_CONFIG_ERR_OK==nRet && pNode->nLength > pNode->nSize)
		nRet = SERVER_CONFIG_ERR_LENGTH;
	
	return nRet;
}

u8 ServerConfig_GetNodeByPos(u32 nPos, ServerConfig_Node* pNode, u8 buf[], u32 *pLen)
{
	u8 nRet = SERVER_CONFIG_ERR_OK;
	u8 readBuf[SERVER_CONFIG_NODE_LEN];

	nRet = ServerConfig_ReadFlash(nPos+gServerConfig_nBaseAddr, readBuf, SERVER_CONFIG_NODE_LEN);

	if(SERVER_CONFIG_ERR_OK==nRet)
		nRet = ServerConfig_DecodeNode(pNode, readBuf);

	if(buf) 
	{
		if(SERVER_CONFIG_ERR_OK==nRet)
			nRet = ServerConfig_ReadFlash(nPos+gServerConfig_nBaseAddr+SERVER_CONFIG_NODE_LEN, buf, pNode->nLength);

		if(SERVER_CONFIG_ERR_OK==nRet && ServerConfig_Crc16Calc(buf, pNode->nLength) != pNode->nContentCrc)
			nRet = SERVER_CONFIG_ERR_CONTENT_CRC;

		*pLen = pNode->nLength;
	}

	return nRet;
}

u8 ServerConfig_SetNodeByPos(u32 nPos, ServerConfig_Node* pNode, const u8 buf[], u32 len)
{
	u8 nRet = SERVER_CONFIG_ERR_OK;
	u8 writeBuf[SERVER_CONFIG_NODE_LEN];

	if(buf) {
		pNode->nContentCrc = ServerConfig_Crc16Calc(buf, len);
		pNode->nLength = len;
	}

	nRet = ServerConfig_EncodeNode(pNode, writeBuf);

	if(SERVER_CONFIG_ERR_OK==nRet)
		nRet = ServerConfig_WriteFlash(nPos+gServerConfig_nBaseAddr, writeBuf, SERVER_CONFIG_NODE_LEN);

	if(buf)
	{
		if(SERVER_CONFIG_ERR_OK==nRet)
			nRet = ServerConfig_WriteFlash(nPos+gServerConfig_nBaseAddr+SERVER_CONFIG_NODE_LEN, buf, len);
	}

	return nRet;
}

u8 ServerConfig_GetContentById(u32 nId, u8 buf[], u32* pLen)
{
	u8 nRet = SERVER_CONFIG_ERR_OK;
	ServerConfig_Node head;
	u32 nPos = 0;

	while(1)
	{
		nRet = ServerConfig_GetNodeByPos(nPos, &head, 0, 0);
		if(nRet != SERVER_CONFIG_ERR_OK)
			break;

		if(head.nId==nId) {
			nRet = ServerConfig_GetNodeByPos(nPos, &head, buf, pLen);
			break;
		}

		if(head.nNextPos==0)
			break;
		
		nPos = head.nNextPos;
	}

	if(SERVER_CONFIG_ERR_OK==nRet && head.nId != nId)
		nRet = SERVER_CONFIG_ERR_NULL;
	
	return nRet;
}

u8 ServerConfig_UpdateContentById(u32 nId, const u8 buf[], u32 len)
{
	u8 nRet = SERVER_CONFIG_ERR_OK;
	ServerConfig_Node head, tmp;
	u32 nPos = 0;

	while(1)
	{
		nRet = ServerConfig_GetNodeByPos(nPos, &head, 0, 0);
		if(nRet != SERVER_CONFIG_ERR_OK)
			break;
		
		if(head.nId==nId)
			break;
		
		if(head.nNextPos==0)
			break;

		nPos = head.nNextPos;
	}

	tmp.nId = nId;
	tmp.nSize = SERVER_CONFIG_TRIM_FLASH_LEN(len);
	tmp.nLength = len;
	tmp.nNextPos = 0;

	if(SERVER_CONFIG_ERR_NULL==nRet) {
		nRet = ServerConfig_SetNodeByPos(0, &tmp, buf, len);
	}

	else if(SERVER_CONFIG_ERR_OK==nRet && head.nId==nId) {
		nRet = ServerConfig_SetNodeByPos(nPos, &head, buf, len);
	}

	else if(SERVER_CONFIG_ERR_OK==nRet && 0==head.nNextPos) {
		u32 nNextPos = nPos + SERVER_CONFIG_NODE_LEN + head.nSize;
		nRet = ServerConfig_SetNodeByPos(nNextPos, &tmp, buf, len);

		if(SERVER_CONFIG_ERR_OK==nRet) {
			head.nNextPos = nNextPos;
			nRet = ServerConfig_SetNodeByPos(nPos, &head, 0, 0);
		}
	}

	return nRet;
}

u8 ServerConfig_CheckAll()
{
	u8 nRet = SERVER_CONFIG_ERR_OK;
	ServerConfig_Node head;
	u32 nPos = 0;
	
	while(1)
	{
		nRet = ServerConfig_GetNodeByPos(nPos, &head, 0, 0);
		if(nRet != SERVER_CONFIG_ERR_OK)
			break;
		
		if(head.nNextPos==0)
			break;
		
		nPos = head.nNextPos;
	}
	
	return nRet;
}

u8 ServerConfig_ClearAll()
{
	u8 writeBuf[SERVER_CONFIG_NODE_LEN];
	memset(writeBuf, 0xFF, SERVER_CONFIG_NODE_LEN);
	return ServerConfig_WriteFlash(gServerConfig_nBaseAddr, writeBuf, SERVER_CONFIG_NODE_LEN);
}

void ServerConfig_SetU8ToBuf(u8 buf[], u8 dat8)
{
	buf[0] = dat8;
}

u8 ServerConfig_GetU8FromBuf(const u8 buf[])
{
	return buf[0];
}

void ServerConfig_SetU16ToBuf(u8 buf[], u16 dat16)
{
	buf[0] = (u8)dat16;
	buf[1] = (u8)(dat16 >> 8);
}

u16 ServerConfig_GetU16FromBuf(const u8 buf[])
{
	u16 dat16 = 0;
	dat16  = buf[0];
	dat16 |= ((u16)buf[1]) << 8;
	return dat16;
}

void ServerConfig_SetU32ToBuf(u8 buf[], u32 dat32)
{
	buf[0] = (u8)dat32;
	buf[1] = (u8)(dat32 >> 8);
	buf[2] = (u8)(dat32 >> 16);
	buf[3] = (u8)(dat32 >> 24);
}

u32 ServerConfig_GetU32FromBuf(const u8 buf[])
{
	u32 dat32 = 0;
	dat32  = buf[0];
	dat32 |= ((u32)buf[1]) << 8;
	dat32 |= ((u32)buf[2]) << 16;
	dat32 |= ((u32)buf[3]) << 24;
	return dat32;
}

void ServerConfig_Crc16Init(void)
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
                remainder = (remainder << 1) ^ 0x1021;  
            }  
            else  
            {  
                remainder = remainder << 1;  
            }  
        }  
        /* Save the result in the table. */  
        gServerConfig_crcTable[dividend] = remainder;  
    }
}

u16 ServerConfig_Crc16Calc(const u8 buf[], u32 len)
{
	u16 crc = 0x0000;
    u32 offset;  
    u8 byte;
	
    /* Divide the message by the polynomial, a byte at a time. */  
    for( offset = 0; offset < len; offset++)  
    {  
        byte = (crc >> 8) ^ buf[offset];  
        crc = gServerConfig_crcTable[byte] ^ (crc << 8);  
    }  
    /* The final remainder is the CRC result. */  
    return (crc ^ 0xFFFF);  
}
