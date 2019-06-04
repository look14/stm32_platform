#include <string.h>
#include "key.h"
#include "gpio_defs.h"
#include "common.h"
#include "system.h"
#include "interrupt_proc_server.h"

u8 g_key1_down_flag;
u8 g_key2_down_flag;

key_struct keyInfos[2] = {
	{ KEY_INDEX1, 0, 0, 0, &g_key1_down_flag, {0,0} }, 
	{ KEY_INDEX2, 0, 0, 0, &g_key2_down_flag, {0,0} },
};

void key_init(void)
{
	SET_GPIO_H(KEY1_GPIO);
	SET_GPIO_H(KEY2_GPIO);

	SET_GPIO_IN(KEY1_GPIO);
	SET_GPIO_IN(KEY2_GPIO);

	g_key1_down_flag = 0;
	g_key2_down_flag = 0;

	g_int_proc_srv_run_exit0  = key1_interrupt_proc;
	g_int_proc_srv_run_exit13 = key2_interrupt_proc;
}

void key1_interrupt_proc(void)
{
	all_key_interrupt_proc(KEY_INDEX1);
}

void key2_interrupt_proc(void)
{
	all_key_interrupt_proc(KEY_INDEX2);
}

u8 any_key_is_down(void)
{
	return ( g_key1_down_flag ||
			 g_key2_down_flag );
}

void all_key_interrupt_proc(u8 index)
{
	key_struct* pKeyInfo;

	if(NULL != (pKeyInfo = get_key_info(index)))
	{
		*pKeyInfo->downFlag = 1;

		if(pKeyInfo->longPressEn)
			get_system_timestamp(&pKeyInfo->timestamp);
	}
}

u8 get_key_pin_status(u8 index)
{
	u8 keyPinStatus = 0;

	switch(index)
	{
	case KEY_INDEX1:	keyPinStatus = READ_GPIO_PIN(KEY1_GPIO);	break;
	case KEY_INDEX2:	keyPinStatus = READ_GPIO_PIN(KEY2_GPIO);	break;
	}

	return keyPinStatus ?0 :1;
}

key_struct* get_key_info(u8 index)
{
	u8 i = 0;
	for(i=0; i<sizeof(keyInfos)/sizeof(key_struct); i++)
	{
		if(keyInfos[i].index == index)
			return &keyInfos[i];
	}

	return NULL;
}

void enable_key_long_press(u8 index, u8 enable)
{
	key_struct* pKeyInfo;

	if(NULL != (pKeyInfo = get_key_info(index)))
	{
		pKeyInfo->longPressEn = enable;
	}
}

u8 is_enable_long_press(u8 index)
{
	key_struct* pKeyInfo;

	if(NULL != (pKeyInfo = get_key_info(index)))
	{
		return pKeyInfo->longPressEn;
	}

	return 0;
}

