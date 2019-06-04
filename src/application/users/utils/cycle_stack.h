#ifndef __CYCLE_STACK_H__
#define __CYCLE_STACK_H__

#include "typedefs.h"

#define CSTACK_TYPE		unsigned long
#define CSTACK_MAX_SIZE	1024

typedef struct _cstack{
	unsigned long	top;
	unsigned long	bottom;
	CSTACK_TYPE		data_buf[CSTACK_MAX_SIZE+1];
}cstack;

void cstack_init(cstack* p_stack);
void cstack_clear(cstack* p_stack);
BOOL cstack_empty(cstack* p_stack);
BOOL cstack_full(cstack* p_stack);
unsigned long cstack_size(cstack* p_stack);
CSTACK_TYPE cstack_top(cstack* p_stack);
CSTACK_TYPE cstack_bottom(cstack* p_stack);
void cstack_pop(cstack* p_stack);
void cstack_push(cstack* p_stack, CSTACK_TYPE elem);

CSTACK_TYPE cstack_at(cstack* p_stack, u32 index);

#endif
