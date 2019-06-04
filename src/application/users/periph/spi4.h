#ifndef __SPI4_H
#define __SPI4_H

#include "typedefs.h"
#include "stm32f10x_gpio.h"

typedef struct __spi4_type{
	GPIO_TypeDef*	csb_gpio;
	u16				csb_pin;
	GPIO_TypeDef*	scl_gpio;
	u16				scl_pin;
	GPIO_TypeDef*	sdo_gpio;
	u16				sdo_pin;
	GPIO_TypeDef*	sdi_gpio;
	u16				sdi_pin;
	u32				delay;

}spi4_type;

__inline void spi4_csb_out(spi4_type *p_spi4);
__inline void spi4_scl_out(spi4_type *p_spi4);
__inline void spi4_sdo_out(spi4_type *p_spi4);
__inline void spi4_sdi_in(spi4_type *p_spi4);

__inline void spi4_csb_1(spi4_type *p_spi4);
__inline void spi4_csb_0(spi4_type *p_spi4);

__inline void spi4_scl_1(spi4_type *p_spi4);
__inline void spi4_scl_0(spi4_type *p_spi4);

__inline void spi4_sdo_1(spi4_type *p_spi4);
__inline void spi4_sdo_0(spi4_type *p_spi4);

__inline u8 spi4_sdi_read(spi4_type *p_spi4);

__inline void spi4_delay(spi4_type *p_spi4);
void spi4_init(spi4_type *p_spi4);

void spi4_send(spi4_type *p_spi4, u8 data8);
u8 spi4_recv(spi4_type *p_spi4);
u8 spi4_send_recv(spi4_type *p_spi4, u8 data8);

void spi4_write(spi4_type *p_spi3, const u8* p_buf, u16 len);
void spi4_read(spi4_type *p_spi3, u8* p_buf, u16 len);
void spi4_write2(spi4_type *p_spi3, u8 cmd, const u8* p_buf, u16 len);
void spi4_read2(spi4_type *p_spi3, u8 cmd, u8* p_buf, u16 len);

#endif
