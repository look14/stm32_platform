//
//  api.c
//  stm32
//
//  Created by look on 2017/1/8.
//  Copyright © 2017年 look. All rights reserved.
//

#include "api.h"

#include "server_comm_protocol.h"
#include "server_comm_control.h"
#include "server_config.h"

#include "stm32.h"

Stm32 stm32;

const char* get_pc_version() {
    const char* strVer = "v1.2";
    return strVer;
}

const char* get_usb_vendor() {
    return CommControl::GetInstance()->GetUsbVendor();
}

const char* get_usb_product() {
    return CommControl::GetInstance()->GetUsbProduct();
}

u8 open_usb() {
    return CommControl::GetInstance()->ReopenUsb();
}

void close_usb() {
    CommControl::GetInstance()->CloseUsb();
}

u8 enter_usb() {
    return CommControl::GetInstance()->EnterUsb();
}

u8 leave_usb() {
    return CommControl::GetInstance()->LeaveUsb();
}

void set_usb_vid(u16 vid) {
    CommControl::GetInstance()->m_nUsbVendorId = vid;
}

void set_usb_pid(u16 pid) {
    CommControl::GetInstance()->m_nUsbProductId = pid;
}

u8 ping() {
    return stm32.Ping();
}

const char* get_firmware() {
    static char strFirmware[32] = "";
    return stm32.GetFirmware(strFirmware) ?"" :strFirmware;
}

u8 soft_reset() {
    return stm32.SoftReset();
}

u8 set_gpio_out(u32 gpio_type, u16 gpio_pin) {
    return CommControl::GetInstance()->SetGpioOut(gpio_type, gpio_pin);
}

u8 set_gpio_in(u32 gpio_type, u16 gpio_pin) {
    return CommControl::GetInstance()->SetGpioIn(gpio_type, gpio_pin);
}

u8 set_gpio_high(u32 gpio_type, u16 gpio_pin) {
    return CommControl::GetInstance()->SetGpioHigh(gpio_type, gpio_pin);
}

u8 set_gpio_low(u32 gpio_type, u16 gpio_pin) {
    return CommControl::GetInstance()->SetGpioLow(gpio_type, gpio_pin);
}

u8 read_gpio(u32 gpio_type, u16 gpio_pin, u8* pres) {
    return CommControl::GetInstance()->ReadGpio(gpio_type, gpio_pin, *pres);
}

u8 config_check() {
    return ServerConfig_CheckAll();
}

u8 config_clear() {
    return ServerConfig_ClearAll();
}

u8 config_get_content(u32 id, u8 buf[], u32 *plen) {
    u8* pbuf = NULL;
    u32 len32 = 0;
    u8 nRet = ServerConfig_GetContentById(id, NULL, &len32);
        
    if(nRet==0) {
        pbuf = new u8[len32];
        nRet = ServerConfig_GetContentById(id, pbuf, &len32);
    }
        
    if(nRet==0) {
        *plen = __min(*plen, len32);
        memcpy(buf, pbuf, *plen);
    }
        
    if(pbuf)
        delete [] pbuf;
    
    return nRet;
}

u8 config_update_content(u32 id, const u8 buf[], u32 len) {
    return ServerConfig_UpdateContentById(id, buf, len);
}

u8 mpu6050_read_reg(u8 addr, u8 buf[], u16 len) {
    return CommControl::GetInstance()->Mpu6050ReadRegs(addr, buf, len);
}

u8 mpu6050_write_reg(u8 addr, const u8 buf[], u16 len) {
    return CommControl::GetInstance()->Mpu6050WriteRegs(addr, buf, len);
}
