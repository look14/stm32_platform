#ifndef __CYCLE_QUEUE_H__
#define __CYCLE_QUEUE_H__

#include "typedefs.h"

#define CQUEUE_TYPE		unsigned long
#define CQUEUE_MAX_SIZE	1024

typedef struct _cqueue{
	unsigned long	front;
	unsigned long	back;
	CQUEUE_TYPE		data_buf[CQUEUE_MAX_SIZE+1];
}cqueue;

void cqueue_init(cqueue* p_queue);
void cqueue_clear(cqueue* p_queue);
BOOL cqueue_empty(cqueue* p_queue);
BOOL cqueue_full(cqueue* p_queue);
unsigned long cqueue_size(cqueue* p_queue);
CQUEUE_TYPE cqueue_front(cqueue* p_queue);
CQUEUE_TYPE cqueue_back(cqueue* p_queue);
void cqueue_pop(cqueue* p_queue);
void cqueue_push(cqueue* p_queue, CQUEUE_TYPE elem);

CQUEUE_TYPE cqueue_at(cqueue* p_queue, u32 index);

#endif
