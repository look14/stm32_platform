#include <string.h>
#include "mpu6050.h"
#include "system.h"
#include "common.h"

void Mpu6050_Init(void) 
{
    i2c_init(&g_i2c_mpu6050);
}

u8 Mpu6050_ReadNReg(u8 addr, u8 buf[], u8 len)
{
    return i2c_read(&g_i2c_mpu6050, addr, buf, len);
}

u8 Mpu6050_WriteNReg(u8 addr, const u8 buf[], u8 len)
{
    return i2c_write(&g_i2c_mpu6050, addr, buf, len);
}

u8 Mpu6050_ReadReg(u8 addr, u8* pdat8)
{
    return i2c_read(&g_i2c_mpu6050, addr, pdat8, 1);
}

u8 Mpu6050_WriteReg(u8 addr, u8 dat8)
{
    return i2c_write(&g_i2c_mpu6050, addr, &dat8, 1);
}

u8 Mpu6050_WriteRegMask(u8 addr, u8 dat8, u8 mask)
{
    u8 value;
	
	if(mask != 0xFF) {
		Mpu6050_ReadReg(addr, &value);
		value &= ~mask;
		dat8  &= mask;
		dat8  |= value;
	} 

	return Mpu6050_WriteReg(addr, dat8);
}

