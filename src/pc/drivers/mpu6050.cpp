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
	WriteReg(MPU6050_PWR_MGMT_1, 0x00);			//��Դ����1���������״̬��ʱ��Ϊ�ڲ�8MHz
	Sleep(200);
	WriteReg(MPU6050_SMPLRT_DIV, 0x04);			//��������125Hz
	WriteReg(MPU6050_CONFIG, 0x06);				//��ʹ��FSYNC,��ʹ����ͬ���������ʣ�DLPF_CFG[2~0],�����������Ƿ�ͨ��DLPF��
														//����ֵ��0x06(5Hz)��ͨ�˲�������5Hz��
														//�Լ��ٶȺ������Ƕ���Ч�����Ƶ��Ϊ1kHz������SMPLRT_DIV��Ƶ�ʻ�׼
	WriteReg(MPU6050_GYRO_CONFIG, 0x10);		//���Բ⣬+-1000��/s
	WriteReg(MPU6050_ACCEL_CONFIG, 0x08);		//���Բ⣬+-4g
	*/

	WriteReg(MPU6050_PWR_MGMT_1, 0x00);       //�������
	Sleep(200);								  //������ߺ����ʱҪ�ϰٺ��룬�����ʼ�����ɹ�

//	WriteReg(MPU6050_PWR_MGMT_1, 0x03);       //ѡʱ��
	WriteReg(MPU6050_SMPLRT_DIV, 0x00);       //�����ǲ����ʣ�1khzЧ������
	WriteReg(MPU6050_CONFIG, 0x06);           //���ٶ�44hz�˲���������42hz�˲�
	WriteReg(MPU6050_GYRO_CONFIG, 0x10);      //������������� +-2000��ÿ��
	WriteReg(MPU6050_ACCEL_CONFIG, 0x08);     //���ٶȶ�������� +-4G

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