#include "spi4.h"

void spi4_csb_out(spi4_type *p_spi4)
{ 
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin   = p_spi4->csb_pin;
  	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
  	GPIO_Init(p_spi4->csb_gpio, &gpio_init);
}

void spi4_scl_out(spi4_type *p_spi4)
{
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin   = p_spi4->scl_pin;
  	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
  	GPIO_Init(p_spi4->scl_gpio, &gpio_init);
}

void spi4_sdo_out(spi4_type *p_spi4)
{
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin   = p_spi4->sdo_pin;
  	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
  	GPIO_Init(p_spi4->sdo_gpio, &gpio_init);
}

void spi4_sdi_in(spi4_type *p_spi4)
{
	GPIO_InitTypeDef gpio_init;
	gpio_init.GPIO_Pin   = p_spi4->sdi_pin;
  	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(p_spi4->sdi_gpio, &gpio_init);
}

void spi4_csb_1(spi4_type *p_spi4)
{
	p_spi4->csb_gpio->BSRR = p_spi4->csb_pin;
}

void spi4_csb_0(spi4_type *p_spi4)
{
	p_spi4->csb_gpio->BRR = p_spi4->csb_pin;
}

void spi4_scl_1(spi4_type *p_spi4)
{
	p_spi4->scl_gpio->BSRR = p_spi4->scl_pin;
}

void spi4_scl_0(spi4_type *p_spi4)
{
	p_spi4->scl_gpio->BRR = p_spi4->scl_pin;
}

void spi4_sdo_1(spi4_type *p_spi4)
{
	p_spi4->sdo_gpio->BSRR = p_spi4->sdo_pin;
}

void spi4_sdo_0(spi4_type *p_spi4)
{
	p_spi4->sdo_gpio->BRR = p_spi4->sdo_pin;
}

u8 spi4_sdi_read(spi4_type *p_spi4) 
{
	return (p_spi4->sdi_gpio->IDR & p_spi4->sdi_pin) ?1 :0;
}

void spi4_delay(spi4_type *p_spi4)
{
	u32 n = p_spi4->delay;
	while(n--);
}

void spi4_init(spi4_type *p_spi4)
{
	spi4_csb_out(p_spi4);
	spi4_scl_out(p_spi4);
	spi4_sdo_out(p_spi4);
	spi4_sdi_in(p_spi4);
	spi4_delay(p_spi4);

	spi4_csb_1(p_spi4);
	spi4_scl_0(p_spi4);
	spi4_sdo_0(p_spi4);
	spi4_delay(p_spi4);
}

void spi4_send(spi4_type *p_spi4, u8 data8)
{
	u8 i;

	for(i=0; i<8; i++)
	{
		spi4_scl_0(p_spi4);

		if(data8 & 0x80)	
			spi4_sdo_1(p_spi4);
		else			
			spi4_sdo_0(p_spi4);

		spi4_delay(p_spi4);

		data8 <<= 1;
		spi4_scl_1(p_spi4);
		spi4_delay(p_spi4);
	}
}

u8 spi4_recv(spi4_type *p_spi4)
{
	u8 i;
	u8 data8 = 0xFF;

	for(i=0; i<8; i++)
	{
		data8 <<= 1;
		spi4_scl_0(p_spi4);
		spi4_delay(p_spi4);

		spi4_scl_1(p_spi4);

		if(spi4_sdi_read(p_spi4))
			data8 |= 0x01;
		else
			data8 &= ~0x01;

		spi4_delay(p_spi4);
	}

	return data8;
}

u8 spi4_send_recv(spi4_type *p_spi4, u8 data8)
{
	u8 i;
	u8 tmp8 = 0xFF;

	for(i=0; i<8; i++)
	{
		tmp8 <<= 1;
		spi4_scl_0(p_spi4);

		if(data8 & 0x80)	
			spi4_sdo_1(p_spi4);
		else			
			spi4_sdo_0(p_spi4);

		spi4_delay(p_spi4);

		data8 <<= 1;
		spi4_scl_1(p_spi4);

		if(spi4_sdi_read(p_spi4))
			tmp8 |= 0x01;
		else
			tmp8 &= ~0x01;

		spi4_delay(p_spi4);
	}

	return tmp8;
}

void spi4_write(spi4_type *p_spi3, const u8* p_buf, u16 len)
{
	u16 i;

   	spi4_sdo_0(p_spi3);
	spi4_scl_0(p_spi3); 
	spi4_csb_0(p_spi3);

	spi4_delay(p_spi3);
	spi4_delay(p_spi3);

	for(i=0; i<len; i++) {
		spi4_send(p_spi3, p_buf[i]);
	}

	spi4_scl_0(p_spi3);
	spi4_delay(p_spi3);
	spi4_delay(p_spi3);

	spi4_csb_1(p_spi3);
	spi4_sdo_0(p_spi3);	
}

void spi4_write2(spi4_type *p_spi3, u8 cmd, const u8* p_buf, u16 len)
{
	u16 i;

   	spi4_sdo_0(p_spi3);
	spi4_scl_0(p_spi3); 
	spi4_csb_0(p_spi3);

	spi4_delay(p_spi3);
	spi4_delay(p_spi3);

	spi4_send(p_spi3, cmd);

	for(i=0; i<len; i++) {
		spi4_send(p_spi3, p_buf[i]);
	}

	spi4_scl_0(p_spi3);
	spi4_delay(p_spi3);
	spi4_delay(p_spi3);

	spi4_csb_1(p_spi3);
	spi4_sdo_0(p_spi3);
}

void spi4_read(spi4_type *p_spi3, u8* p_buf, u16 len)
{
	u16 i;

	spi4_sdo_0(p_spi3);
	spi4_scl_0(p_spi3); 
	spi4_csb_0(p_spi3);

	spi4_delay(p_spi3);
	spi4_delay(p_spi3);

	for(i=0; i<len; i++) {
		p_buf[i] = spi4_recv(p_spi3);
	}

	spi4_scl_0(p_spi3);
	spi4_delay(p_spi3);
	spi4_delay(p_spi3);

	spi4_csb_1(p_spi3);
	spi4_sdo_0(p_spi3);
}

void spi4_read2(spi4_type *p_spi3, u8 cmd, u8* p_buf, u16 len)
{
	u16 i;

	spi4_sdo_0(p_spi3);
	spi4_scl_0(p_spi3); 
	spi4_csb_0(p_spi3);

	spi4_delay(p_spi3);
	spi4_delay(p_spi3);

	spi4_send(p_spi3, cmd);

	for(i=0; i<len; i++) {
		p_buf[i] = spi4_recv(p_spi3);
	}

	spi4_scl_0(p_spi3);
	spi4_delay(p_spi3);
	spi4_delay(p_spi3);

	spi4_csb_1(p_spi3);	
	spi4_sdo_0(p_spi3);
}
