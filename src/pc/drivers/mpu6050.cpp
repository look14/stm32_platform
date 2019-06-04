#include "stdafx.h"
#include "mpu6050.h"
#include "server_comm_control.h"
#include "common.h"

Mpu6050::Mpu6050()
{
	LARGE_INTEGER liPerfFreq;
	QueryPerformanceFrequency(&liPerfFreq); 
	m_fUsDivisor      = (double)1000000.0 / liPerfFreq.QuadPart;
	m_fAccelPrecision = 0;
	m_fGyroPrecision  = 0;
}

Mpu6050::~Mpu6050()
{
}

double Mpu6050::GetUsTimeCounter()
{
	LARGE_INTEGER liPerfNow;
	QueryPerformanceCounter(&liPerfNow);
	return (double)liPerfNow.QuadPart * m_fUsDivisor;
}

u8 Mpu6050::WriteReg(u8 addr, u8 dat)
{	
	return CommControl::GetInstance()->Mpu6050WriteRegs(addr, &dat, 1);
}

u8 Mpu6050::WriteRegMask(u8 addr, u8 dat, u8 mask)
{
	u8 value;
	
	if(mask!=0xFF)
	{
		CommControl::GetInstance()->Mpu6050ReadRegs(addr, &value, 1);
		value &= ~mask;
		dat   &= mask;
		dat   |= value;
	}
	
	return CommControl::GetInstance()->Mpu6050WriteRegs(addr, &dat, 1);
}

u8 Mpu6050::ReadReg(u8 addr, u8 *dat)
{
	u8 nRet;
	nRet = CommControl::GetInstance()->Mpu6050ReadRegs(addr, dat, 1);
	return nRet;
}

u8 Mpu6050::WriteNReg(u8 addr, const u8* pdat, u8 len)
{
	return CommControl::GetInstance()->Mpu6050WriteRegs(addr, pdat, len);
}

u8 Mpu6050::ReadNReg(u8 addr, u8* pdat, u8 len)
{
	return CommControl::GetInstance()->Mpu6050ReadRegs(addr, pdat, len);
}

u8 Mpu6050::GetDataU8(u8 addr)
{
	u8 dat8;
	ReadReg(addr, &dat8);
	return dat8;
}

s16 Mpu6050::GetDataS16(u8 addr)
{
	u8 buf[2] = { 0, 0 };
	ReadNReg(addr, buf, 2);
	return ( (s16)buf[0] << 8 ) | ( (s16)buf[1] );
}

BOOL Mpu6050::Config(void)
{
	u8 dat8;

	ReadReg(MPU6050_WHO_AM_I, &dat8);
	if(0x68 != ( 0x7E & dat8) )
	{
		return FALSE;
	}

	WriteReg(MPU6050_PWR_MGMT_1, 0x80);

	/*
	WriteReg(MPU6050_PWR_MGMT_1, 0x00);			//电源管理1，解除休眠状态，时钟为内部8MHz
	Sleep(200);
	WriteReg(MPU6050_SMPLRT_DIV, 0x04);			//采样速率125Hz
	WriteReg(MPU6050_CONFIG, 0x06);				//不使能FSYNC,不使用外同步采样速率；DLPF_CFG[2~0],设置任意轴是否通过DLPF，
														//典型值：0x06(5Hz)低通滤波器带宽5Hz，
														//对加速度和陀螺仪都有效，输出频率为1kHz，决定SMPLRT_DIV的频率基准
	WriteReg(MPU6050_GYRO_CONFIG, 0x10);		//不自测，+-1000°/s
	WriteReg(MPU6050_ACCEL_CONFIG, 0x08);		//不自测，+-4g
	*/

	WriteReg(MPU6050_PWR_MGMT_1, 0x00);       //解除休眠
	Sleep(200);								  //解除休眠后的延时要上百毫秒，否则初始化不成功

//	WriteReg(MPU6050_PWR_MGMT_1, 0x03);       //选时钟
	WriteReg(MPU6050_SMPLRT_DIV, 0x00);       //陀螺仪采样率，1khz效果不错
	WriteReg(MPU6050_CONFIG, 0x06);           //加速度44hz滤波，陀螺仪42hz滤波
	WriteReg(MPU6050_GYRO_CONFIG, 0x10);      //陀螺仪最大量程 +-2000度每秒
	WriteReg(MPU6050_ACCEL_CONFIG, 0x08);     //加速度度最大量程 +-4G

	m_fGyroPrecision  = (double)2000.0 / 65536.0;
	m_fAccelPrecision = (double)8.0 * MPU6050_GRAVITY_ACCEL / 65536.0;

	return TRUE;
}

BOOL Mpu6050::GetCurrentData(double &fAccelX, 
	double &fAccelY, 
	double &fAccelZ, 
	double &fGyroX, 
	double &fGyroY, 
	double &fGyroZ, 
	double &fTemperature)
{
	u8 buf[14];
	memset(buf, 0, sizeof(buf));

	if(0 == ReadNReg(MPU6050_ACCEL_XOUT_H, buf, sizeof(buf)))
	{
		fAccelX = m_fAccelPrecision * MPU6050_GET_S16(&buf[0]);
		fAccelY = m_fAccelPrecision * MPU6050_GET_S16(&buf[2]);
		fAccelZ = m_fAccelPrecision * MPU6050_GET_S16(&buf[4]);

		fGyroX  = m_fGyroPrecision * MPU6050_GET_S16(&buf[8]);
		fGyroY  = m_fGyroPrecision * MPU6050_GET_S16(&buf[10]);
		fGyroZ  = m_fGyroPrecision * MPU6050_GET_S16(&buf[12]);

		fTemperature = (double)MPU6050_GET_S16(&buf[6]) / 340.0 + 36.53;
	}
	else
		return FALSE;

	return TRUE;
}