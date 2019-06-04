#ifndef __KEY_H
#define __KEY_H

#include "typedefs.h"
#include "time_server.h"

typedef struct __key_struct {
	u8 index;
	u8 longPressEn;
	u8 longPressFlag;
	u8 longPressCnt;
	u8 *downFlag;
	system_timestamp timestamp;
} key_struct;


#define KEY_ALL			0
#define KEY_INDEX1		1
#define KEY_INDEX2		2

extern u8 g_key1_down_flag;
extern u8 g_key2_down_flag;

extern key_struct keyInfos[2];

void key_init(void);
void key1_interrupt_proc(void);
void key2_interrupt_proc(void);
u8 any_key_is_down(void);
void all_key_interrupt_proc(u8 index);

u8 get_key_pin_status(u8 index);
key_struct* get_key_info(u8 index);
void enable_key_long_press(u8 index, u8 enable);
u8 is_enable_long_press(u8 index);


#endif
