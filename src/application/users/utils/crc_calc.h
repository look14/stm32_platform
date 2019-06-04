#ifndef __CRC_CALC_H
#define __CRC_CALC_H

#include "typedefs.h"

#define CRC_CALC_POLY			0x3D65//0x1021
#define CRC_CALC_OPTIMIZE_EN	1

#if !CRC_CALC_OPTIMIZE_EN
void crc16_init(void);
u16 crc16_calc(u8 buf[], u16 len);
#endif

#if CRC_CALC_OPTIMIZE_EN
void crc16_init(void);
u16 crc16_calc(u8 buf[], u16 len);
#endif

u16 crc16_ibm_calc(const u8 buf[], u16 len);
u16 crc16_ccitt_calc(const u8 buf[], u16 len);

#endif
