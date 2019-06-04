#ifndef __COMMON_H
#define __COMMON_H

#include "typedefs.h"

void set_u16_to_buf(u8 buf[], u16 dat16);
u16 get_u16_from_buf(const u8 buf[]);

void set_u32_to_buf(u8 buf[], u32 dat32);
u32 get_u32_from_buf(const u8 buf[]);

u32 calc_get_bits(const u32 *src, u8 bHigh, u8 bLow);
void calc_set_bits(u32 *dest, u8 bHigh, u8 bLow, u32 src);

#endif
