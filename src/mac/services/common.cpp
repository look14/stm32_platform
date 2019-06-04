#include "stdafx.h"
#include "common.h"

void set_u16_to_buf(u8 buf[], u16 dat16)
{
	buf[0] = (u8)dat16;
	buf[1] = (u8)(dat16 >> 8);
}

u16 get_u16_from_buf(const u8 buf[])
{
	u16 dat16 = 0;
	dat16  = buf[0];
	dat16 |= ((u16)buf[1]) << 8;
	return dat16;
}

void set_u32_to_buf(u8 buf[], u32 dat32)
{
	buf[0] = (u8)dat32;
	buf[1] = (u8)(dat32 >> 8);
	buf[2] = (u8)(dat32 >> 16);
	buf[3] = (u8)(dat32 >> 24);
}

u32 get_u32_from_buf(const u8 buf[])
{
	u32 dat32 = 0;
	dat32  = buf[0];
	dat32 |= ((u32)buf[1]) << 8;
	dat32 |= ((u32)buf[2]) << 16;
	dat32 |= ((u32)buf[3]) << 24;
	return dat32;
}

u32 calc_get_bits(const u32 *src, u8 bHigh, u8 bLow)
{
	// return src[bHigh:bLow]
	
	u32 ret		= *src;
	u32 mask	= 0;
	u8 len		= bHigh - bLow + 1;
	u8 i;
	
	for(i=0; i<len; i++)	{	mask <<= 1;	mask  |= 1; }
	for(i=0; i<bLow; i++)	ret >>= 1;
	ret &= mask;
	
	return ret;
}

void calc_set_bits(u32 *dest, u8 bHigh, u8 bLow, u32 src)
{
	// dest[bHigh:bLow] = src[bHigh:bLow]
	
	u32 mask	= 0;
	u8 len		= bHigh - bLow + 1;
	u8 i;
	
	for(i=0; i<len; i++)	{ mask <<= 1;	mask  |= 1; }
	
	mask  <<= bLow;
	src   <<= bLow;
	src    &= mask;
	*dest  &= ~mask;
	*dest  |= src;
}
