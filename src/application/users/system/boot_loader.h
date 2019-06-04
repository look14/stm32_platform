#ifndef __BOOT_LOADER_H
#define __BOOT_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "typedefs.h"

#define MODE_APP_FW_INVALID       0x00
#define MODE_APP_FW_VALID         0x01

extern const u8 g_APP_Flag[];

u8 Bootloader_Check_App_FW_Validity(void);
void Bootloader_flag_erase(void);
void Bootloader_flag_write(void);
void Bootloader_closeBsp(void);

u8 Flash_JumpToExeCode(void);


#ifdef __cplusplus
}
#endif

#endif
