//
//  api.h
//  stm32
//
//  Created by look on 2017/1/8.
//  Copyright © 2017年 look. All rights reserved.
//

#ifndef api_h
#define api_h

#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

    const char* get_pc_version();
    const char* get_usb_vendor();
    const char* get_usb_product();
    u8 open_usb();
    void close_usb();
    u8 enter_usb();
    u8 leave_usb();
    void set_usb_vid(u16 vid);
    void set_usb_pid(u16 pid);
    u8 ping();
    const char* get_firmware();
    u8 soft_reset();
    u8 set_gpio_out(u32 gpio_type, u16 gpio_pin);
    u8 set_gpio_in(u32 gpio_type, u16 gpio_pin);
    u8 set_gpio_high(u32 gpio_type, u16 gpio_pin);
    u8 set_gpio_low(u32 gpio_type, u16 gpio_pin);
    u8 read_gpio(u32 gpio_type, u16 gpio_pin, u8* pres);
    u8 config_check();
    u8 config_clear();
    u8 config_get_content(u32 id, u8 buf[], u32* plen);
    u8 config_update_content(u32 id, const u8 buf[], u32 len);
    u8 mpu6050_read_reg(u8 addr, u8 buf[], u16 len);
    u8 mpu6050_write_reg(u8 addr, const u8 buf[], u16 len);
    
#ifdef __cplusplus
}
#endif

#endif /* api_h */
