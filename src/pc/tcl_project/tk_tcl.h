#ifndef __TK_TCL_H
#define __TK_TCL_H

#include "typedefs.h"
#include "C:\\Tcl\\include\\tcl.h"

#pragma comment(lib, "C:\\Tcl\\lib\\tcl86")

int TclCmdProc(ClientData clientData, Tcl_Interp *interp, int objc, char * CONST * objv);

extern "C" __declspec(dllexport) int Stmusb_Init(Tcl_Interp *interp);


#endif
