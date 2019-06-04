#include "i2c.h"

void i2c_sck_out(i2c_type* ptype)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin   = ptype->sck_pin;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  	GPIO_Init(ptype->sck_type, &GPIO_InitStructure);
}

void i2c_sda_out(i2c_type* ptype)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin   = ptype->sda_pin;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
  	GPIO_Init(ptype->sda_type, &GPIO_InitStructure);
}

void i2c_sda_in(i2c_type* ptype)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin   = ptype->sda_pin;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(ptype->sda_type, &GPIO_InitStructure);
}

void i2c_sck_1(i2c_type* ptype)
{
	ptype->sck_type->BSRR = ptype->sck_pin;
}

void i2c_sck_0(i2c_type* ptype)
{
	ptype->sck_type->BRR = ptype->sck_pin;
}

void i2c_sda_1(i2c_type* ptype)
{
	ptype->sda_type->BSRR = ptype->sda_pin;
}

void i2c_sda_0(i2c_type* ptype)
{
	ptype->sda_type->BRR = ptype->sda_pin;
}

u8 i2c_sda_read(i2c_type* ptype)
{
	return (ptype->sda_type->IDR & ptype->sda_pin) ?1 :0;
}

void i2c_delay(i2c_type* ptype)
{
	u16 n = ptype->delay;
	while(n--);
}

void i2c_init(i2c_type* ptype)
{
	i2c_sck_out(ptype);
	i2c_sda_out(ptype);
	i2c_delay(ptype);
	
	i2c_sck_0(ptype);
	i2c_sda_1(ptype);
	i2c_delay(ptype);
}

u8 i2c_start(i2c_type* ptype)
{
	i2c_sda_1(ptype);
	i2c_sck_1(ptype);
	i2c_delay(ptype);
	
	if( !i2c_sda_read(ptype) )	
		return I2C_BUS_BUSY;
	
	i2c_sda_0(ptype);
	i2c_delay(ptype);
	
	if( i2c_sda_read(ptype) )	
		return I2C_BUS_ERROR;
	
	i2c_sda_0(ptype);
	i2c_delay(ptype);
	
	return I2C_READY;
}

void i2c_stop(i2c_type* ptype)
{
	i2c_sck_0(ptype);	i2c_delay(ptype);
	i2c_sda_0(ptype);	i2c_delay(ptype);
	i2c_sck_1(ptype);	i2c_delay(ptype);
	i2c_sda_1(ptype);	i2c_delay(ptype);
}

void i2c_ack(i2c_type* ptype)
{
	i2c_sck_0(ptype);	i2c_delay(ptype);
	i2c_sda_0(ptype);	i2c_delay(ptype);
	i2c_sck_1(ptype);	i2c_delay(ptype);
	i2c_sck_0(ptype);	i2c_delay(ptype);
}

void i2c_nack(i2c_type* ptype)
{
	i2c_sck_0(ptype);	i2c_delay(ptype);
	i2c_sda_1(ptype);	i2c_delay(ptype);
	i2c_sck_1(ptype);	i2c_delay(ptype);
	i2c_sck_0(ptype);	i2c_delay(ptype);
}

u8 i2c_waitack(i2c_type* ptype)
{
	i2c_sck_0(ptype);	i2c_delay(ptype);
	i2c_sda_1(ptype);	i2c_delay(ptype);
	i2c_sck_1(ptype);	i2c_delay(ptype);
	
	if( i2c_sda_read(ptype) ) {
		i2c_sck_0(ptype);
		return I2C_NACK;
	}
	else {
		i2c_sck_0(ptype);
		return I2C_ACK;
	}
}

u8 i2c_send(i2c_type* ptype, u8 data8)
{
	u8 i;
	
	for(i=0; i<8; i++)
	{
		i2c_sck_0(ptype);
		i2c_delay(ptype);

		if(data8 & 0x80)	
			i2c_sda_1(ptype);
		else				
			i2c_sda_0(ptype);

		data8 <<= 1;
		i2c_delay(ptype);
		
		i2c_sck_1(ptype);
		i2c_delay(ptype);
	}

	i2c_sck_0(ptype);

	return i2c_waitack(ptype);
}

u8 i2c_recv(i2c_type* ptype)
{
	u8 i;
	u8 data8 = 0;

	i2c_sda_1(ptype);

	for(i=0; i<8; i++)
	{
		data8 <<= 1;
		
		i2c_sck_0(ptype);
		i2c_delay(ptype);
		
		i2c_sck_1(ptype);
		i2c_delay(ptype);
		
		data8 |= i2c_sda_read(ptype);

	}

	i2c_sck_0(ptype);

	return data8;
}

u8 i2c_write(i2c_type* ptype, u8 addr, const u8* pbuf, u8 len)
{
	u8 res;
	
	if( (res = i2c_start(ptype)) != I2C_READY) {
		return res;
	}

	if( i2c_send(ptype, ptype->dev_addr & 0xFE) != I2C_ACK ) {
		i2c_stop(ptype);
		return I2C_NACK;
	}

	i2c_send(ptype, addr);

	while(len--)
	{
		i2c_send(ptype, *pbuf++);	
	}

	i2c_stop(ptype);

	return I2C_OK;
}

u8 i2c_read(i2c_type* ptype, u8 addr, u8* pbuf, u8 len)
{
	u8 res;
	
	if( (res = i2c_start(ptype)) != I2C_READY) {
		return res;
	}

	if( i2c_send(ptype, ptype->dev_addr & 0xFE) != I2C_ACK ) {
		i2c_stop(ptype);
		return I2C_NACK;
	}

	i2c_send(ptype, addr);
	i2c_start(ptype);
	i2c_send(ptype, ptype->dev_addr | 0x01);

	while(len)
	{
		*pbuf++ = i2c_recv(ptype);

		if(len == 1)	i2c_nack(ptype);
		else			i2c_ack(ptype);
		
		len--;	
	}

	i2c_stop(ptype);

	return I2C_OK;
}
