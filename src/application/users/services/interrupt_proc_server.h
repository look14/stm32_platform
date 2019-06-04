#ifndef __INTERRUPT_PROC_SERVER_H
#define __INTERRUPT_PROC_SERVER_H

#include "typedefs.h"

typedef void (*p_int_proc_srv_run)(void);

extern p_int_proc_srv_run g_int_proc_srv_run_exit0;
extern p_int_proc_srv_run g_int_proc_srv_run_exit1;
extern p_int_proc_srv_run g_int_proc_srv_run_exit2;
extern p_int_proc_srv_run g_int_proc_srv_run_exit3;
extern p_int_proc_srv_run g_int_proc_srv_run_exit4;
extern p_int_proc_srv_run g_int_proc_srv_run_exit5;
extern p_int_proc_srv_run g_int_proc_srv_run_exit6;
extern p_int_proc_srv_run g_int_proc_srv_run_exit7;
extern p_int_proc_srv_run g_int_proc_srv_run_exit8;
extern p_int_proc_srv_run g_int_proc_srv_run_exit9;
extern p_int_proc_srv_run g_int_proc_srv_run_exit10;
extern p_int_proc_srv_run g_int_proc_srv_run_exit11;
extern p_int_proc_srv_run g_int_proc_srv_run_exit12;
extern p_int_proc_srv_run g_int_proc_srv_run_exit13;
extern p_int_proc_srv_run g_int_proc_srv_run_exit14;
extern p_int_proc_srv_run g_int_proc_srv_run_exit15;

void int_proc_srv_init(void);


#endif
