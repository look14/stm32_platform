# -- coding: utf-8 -- 

GPIOA						= 0x40010800
GPIOB						= 0x40010C00
GPIOC						= 0x40011000
GPIOD						= 0x40011400
GPIOE						= 0x40011800
GPIOF						= 0x40011C00
GPIOG						= 0x40012000
	
GPIO_Pin_0  				= 0x0001
GPIO_Pin_1  				= 0x0002
GPIO_Pin_2  				= 0x0004
GPIO_Pin_3  				= 0x0008
GPIO_Pin_4  				= 0x0010
GPIO_Pin_5  				= 0x0020
GPIO_Pin_6  				= 0x0040
GPIO_Pin_7  				= 0x0080
GPIO_Pin_8  				= 0x0100
GPIO_Pin_9  				= 0x0200
GPIO_Pin_10 				= 0x0400
GPIO_Pin_11 				= 0x0800
GPIO_Pin_12 				= 0x1000
GPIO_Pin_13 				= 0x2000
GPIO_Pin_14 				= 0x4000
GPIO_Pin_15 				= 0x8000
GPIO_Pin_All				= 0xFFFF


# set_gpio_out	$GPIOA	$GPIO_Pin_0
# set_gpio_in	$GPIOA	$GPIO_Pin_0
# set_gpio_high	$GPIOA	$GPIO_Pin_0
# set_gpio_low	$GPIOA	$GPIO_Pin_0
# read_gpio		$GPIOA	$GPIO_Pin_0
