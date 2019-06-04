#ifndef __SERVER_CONFIG_H
#define __SERVER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "typedefs.h"

typedef struct __ServerConfig_Node {
	u32 nId;
	u32 nSize;
	u32 nLength;
	u32 nNextPos;
	u16 nContentCrc;
	u16 nNodeCrc;
}ServerConfig_Node;

typedef u8(*pServerConfig_ReadFlash)(u32 addr, u8 buf[], u32 len);
typedef u8(*pServerConfig_WriteFlash)(u32 addr, const u8 buf[], u32 len);

#define SERVER_CONFIG_NODE_LEN				20

#define SERVER_CONFIG_ERR_OK				0
#define SERVER_CONFIG_ERR_NULL				1
#define SERVER_CONFIG_ERR_READ_FLASH		2
#define SERVER_CONFIG_ERR_WRITE_FLASH		3
#define SERVER_CONFIG_ERR_SIZE				4
#define SERVER_CONFIG_ERR_LENGTH			5
#define SERVER_CONFIG_ERR_NEXT_POS			6
#define SERVER_CONFIG_ERR_NODE_CRC			7
#define SERVER_CONFIG_ERR_CONTENT_CRC		8

#define SERVER_CONFIG_TRIM_FLASH_LEN(len)	((len/4)*4 + ((len%4) ?4 :0))	// 对齐到4的倍数

u8 ServerConfig_Init(u32 nBaseAddr, pServerConfig_ReadFlash pReadFlash, pServerConfig_WriteFlash pWriteFlash);

u8 ServerConfig_DecodeNode(ServerConfig_Node* pNode, const u8 buf[]);
u8 ServerConfig_EncodeNode(const ServerConfig_Node* pNode, u8 buf[]);

u8 ServerConfig_GetNodeByPos(u32 nPos, ServerConfig_Node* pNode, u8 buf[], u32 *pLen);
u8 ServerConfig_SetNodeByPos(u32 nPos, ServerConfig_Node* pNode, const u8 buf[], u32 len);

u8 ServerConfig_GetContentById(u32 nId, u8 buf[], u32* pLen);
u8 ServerConfig_UpdateContentById(u32 nId, const u8 buf[], u32 len);

u8 ServerConfig_CheckAll(void);
u8 ServerConfig_ClearAll(void);

void ServerConfig_SetU8ToBuf(u8 buf[], u8 dat8);
u8   ServerConfig_GetU8FromBuf(const u8 buf[]);
void ServerConfig_SetU16ToBuf(u8 buf[], u16 dat16);
u16  ServerConfig_GetU16FromBuf(const u8 buf[]);
void ServerConfig_SetU32ToBuf(u8 buf[], u32 dat32);
u32  ServerConfig_GetU32FromBuf(const u8 buf[]);

void ServerConfig_Crc16Init(void);
u16  ServerConfig_Crc16Calc(const u8 buf[], u32 len);

#ifdef __cplusplus
}
#endif

#endif
