#include "cycle_stack.h"
#include <string.h>

void cstack_init(cstack* p_stack)
{
	p_stack->top	= 0;
	p_stack->bottom	= 0;

	memset(p_stack->data_buf, 0, CSTACK_MAX_SIZE+1);
}

void cstack_clear(cstack* p_stack)
{
	p_stack->top	= 0;
	p_stack->bottom	= 0;
}

BOOL cstack_empty(cstack* p_stack)
{
	return p_stack->top==p_stack->bottom ?TRUE :FALSE;
}

BOOL cstack_full(cstack* p_stack)
{
	return ((p_stack->top+1) % (CSTACK_MAX_SIZE+1))==p_stack->bottom ?TRUE :FALSE;
}

unsigned long cstack_size(cstack* p_stack)
{
	return (p_stack->top >= p_stack->bottom)
		?(p_stack->top - p_stack->bottom)
		:((CSTACK_MAX_SIZE+1) + p_stack->top - p_stack->bottom);
}

CSTACK_TYPE cstack_bottom(cstack* p_stack)
{
	return p_stack->data_buf[p_stack->bottom];
}

CSTACK_TYPE cstack_top(cstack* p_stack)
{
	return p_stack->data_buf[(p_stack->top==0 ?(CSTACK_MAX_SIZE) :(p_stack->top-1))];
}

void cstack_pop(cstack* p_stack)
{
	if(p_stack->top != p_stack->bottom) {
		p_stack->top = p_stack->top==0 ?(CSTACK_MAX_SIZE) :(p_stack->top-1);
	}
}

void cstack_push(cstack* p_stack, CSTACK_TYPE elem)
{
	p_stack->data_buf[p_stack->top] = elem;
	p_stack->top = (p_stack->top+1) % (CSTACK_MAX_SIZE+1);
	if(p_stack->top == p_stack->bottom) {
		p_stack->bottom = (p_stack->bottom+1)%(CSTACK_MAX_SIZE+1);
	}
}

CSTACK_TYPE cstack_at(cstack* p_stack, u32 index)
{
	index  = ( (p_stack->top>index) ?(p_stack->top-1) :(p_stack->top+CSTACK_MAX_SIZE) ) - index;
	index %= CSTACK_MAX_SIZE+1;

	return p_stack->data_buf[index];
}
