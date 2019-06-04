#include "cycle_queue.h"
#include <string.h>

void cqueue_init(cqueue* p_queue)
{
	p_queue->front	= 0;
	p_queue->back	= 0;

	memset(p_queue->data_buf, 0, CQUEUE_MAX_SIZE+1);
}

void cqueue_clear(cqueue* p_queue)
{
	p_queue->front	= 0;
	p_queue->back	= 0;
}

BOOL cqueue_empty(cqueue* p_queue)
{
	return p_queue->front==p_queue->back ?TRUE :FALSE;
}

BOOL cqueue_full(cqueue* p_queue)
{
	return ((p_queue->back+1) % (CQUEUE_MAX_SIZE+1))==p_queue->front ?TRUE :FALSE;
}

unsigned long cqueue_size(cqueue* p_queue)
{
	return (p_queue->back >= p_queue->front)
		?(p_queue->back - p_queue->front)
		:((CQUEUE_MAX_SIZE+1) + p_queue->back - p_queue->front);
}

CQUEUE_TYPE cqueue_front(cqueue* p_queue)
{
	return p_queue->data_buf[p_queue->front];
}

CQUEUE_TYPE cqueue_back(cqueue* p_queue)
{
	return p_queue->data_buf[(p_queue->back==0 ?(CQUEUE_MAX_SIZE) :(p_queue->back-1))];
}

void cqueue_pop(cqueue* p_queue)
{
	if(p_queue->back != p_queue->front) {
		p_queue->front = (p_queue->front+1)%(CQUEUE_MAX_SIZE+1);
	}
}

void cqueue_push(cqueue* p_queue, CQUEUE_TYPE elem)
{
	p_queue->data_buf[p_queue->back] = elem;
	p_queue->back = (p_queue->back+1) % (CQUEUE_MAX_SIZE+1);
	if(p_queue->back == p_queue->front) {
		p_queue->front = (p_queue->front+1)%(CQUEUE_MAX_SIZE+1);
	}
}

CQUEUE_TYPE cqueue_at(cqueue* p_queue, u32 index)
{
	index += p_queue->front;
	index %= CQUEUE_MAX_SIZE+1;

	return p_queue->data_buf[index];
}
