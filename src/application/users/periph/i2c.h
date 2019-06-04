#ifndef __I2C_H
#define __I2C_H

#include "typedefs.h"
#include "gpio_defs.h"

#define I2C_OK					0
#define I2C_ACK     			0
#define I2C_READY   			0
#define I2C_NACK   				1
#define I2C_BUS_BUSY    		2
#define I2C_BUS_ERROR   		3
#define I2C_RETRY_COUNT 		3

typedef struct __i2c_type{
	GPIO_TypeDef* sck_type;
	GPIO_TypeDef* sda_type;
	u16 sck_pin;
	u16 sda_pin;
	u16 delay;
	u8 dev_addr;
}i2c_type;

__inline void i2c_sck_out(i2c_type* ptype);
__inline void i2c_sda_out(i2c_type* ptype);
__inline void i2c_sda_in(i2c_type* ptype);

__inline void i2c_sck_1(i2c_type* ptype);
__inline void i2c_sck_0(i2c_type* ptype);

__inline void i2c_sda_1(i2c_type* ptype);
__inline void i2c_sda_0(i2c_type* ptype);
__inline u8 i2c_sda_read(i2c_type* ptype);

__inline void i2c_delay(i2c_type* ptype);
void i2c_init(i2c_type* ptype);

__inline u8 i2c_start(i2c_type* ptype);
__inline void i2c_stop(i2c_type* ptype);
__inline void i2c_ack(i2c_type* ptype);
__inline void i2c_nack(i2c_type* ptype);
__inline u8 i2c_waitack(i2c_type* ptype);
__inline u8 i2c_send(i2c_type* ptype, u8 data8); 
__inline u8 i2c_recv(i2c_type* ptype);

u8 i2c_write(i2c_type* ptype, u8 addr, const u8* pbuf, u8 len);
u8 i2c_read(i2c_type* ptype, u8 addr, u8* pbuf, u8 len);

#endif
