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

static unsigned short inv_row_2_scale(const signed char *row)
{
	unsigned short b;

	if (row[0] > 0)
		b = 0;
	else if (row[0] < 0)
		b = 4;
	else if (row[1] > 0)
		b = 1;
	else if (row[1] < 0)
		b = 5;
	else if (row[2] > 0)
		b = 2;
	else if (row[2] < 0)
		b = 6;
	else
		b = 7;		// error
	return b;
}

/** Converts an orientation matrix made up of 0,+1,and -1 to a scalar representation.
* @param[in] mtx Orientation matrix to convert to a scalar.
* @return Description of orientation matrix. The lowest 2 bits (0 and 1) represent the column the one is on for the
* first row, with the bit number 2 being the sign. The next 2 bits (3 and 4) represent
* the column the one is on for the second row with bit number 5 being the sign.
* The next 2 bits (6 and 7) represent the column the one is on for the third row with
* bit number 8 being the sign. In binary the identity matrix would therefor be:
* 010_001_000 or 0x88 in hex.
*/
unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx)
{

    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}
