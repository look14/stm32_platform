#ifndef __TYPEDEFS_H
#define __TYPEDEFS_H

#include "stm32f10x.h"
#include "usb_type.h"

/*
typedef unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned long 		u32;
typedef signed char	 		s8;			
typedef signed short 		s16;			
typedef signed long	 		s32;
*/	

#ifndef BOOL
#define BOOL    u8
#endif

/*#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif*/

#define LOBYTE(x)			( (u8)(x) )	
#define HIBYTE(x)			( (u8)(x >> 8) )	

#define GET_U16(buf)		( (u16)(*buf) | ( (u16)(*(buf+1)) << 8 ) )

#endif
