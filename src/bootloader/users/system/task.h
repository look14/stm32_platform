#ifndef __TASK_H
#define __TASK_H

#include "typedefs.h"

void task_init(void);
void task_run(void);

u8 usb_thread_proc(void);
u8 test_thread_proc(void);


#endif
