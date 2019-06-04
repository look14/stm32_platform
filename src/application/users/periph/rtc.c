#include "rtc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_rtc.h"

#include <string.h>

volatile rtc_date_time g_rtc_date_time;
const u8 g_rtc_mon_table[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
const u8 g_rtc_table_week[12] = {0,3,3,6,1,4,6,2,5,0,3,5}; 

void rtc_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
																	//ʹ��PWR��BKP����ʱ��   		
	PWR_BackupAccessCmd(ENABLE);									//ʹ��RTC�ͺ󱸼Ĵ������� 
	
	if(BKP_ReadBackupRegister(BKP_DR1)!=0x5555)						//��ָ���ĺ󱸼Ĵ����ж������ݣ��ж��Ƿ�Ϊ��һ������
	{	
      	//printf("ʱ�����á�����\r\n");																
		BKP_DeInit();												//������BKP��ȫ���Ĵ�������Ϊȱʡֵ 	
		RCC_LSEConfig(RCC_LSE_ON);									//ʹ���ⲿ����ʱ�� 32.768KHz
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)			//���ָ����RCC��־λ�������,�ȴ����پ������
  		{}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);						//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��    
		RCC_RTCCLKCmd(ENABLE);										//ʹ��RTCʱ��  
		RTC_WaitForSynchro();										//�ȴ�RTC�Ĵ���(RTC_CNT,RTC_ALR��RTC_PRL)��RTC APBʱ��ͬ��
		RTC_WaitForLastTask();										//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		RTC_ITConfig(RTC_IT_SEC, ENABLE);							//ʹ��RTC���ж�
		RTC_WaitForLastTask();										//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		RTC_SetPrescaler(32767); 									//����RTCԤ��Ƶ��ֵ  RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
		RTC_WaitForLastTask();										//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		rtc_set_time();												//ʱ������	
    	BKP_WriteBackupRegister(BKP_DR1, 0x5555);					//��ָ���ĺ󱸼Ĵ�����д���û���������0X5555���жϱ�־										
	}																 	
	else															//���ǵ�һ������ ������ʱ
	{
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)			//���ָ����RCC��־λ�������:POR/PDR��λ
		{
      		;//printf("�ϵ縴λ������\r\n");
		}											
		else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)		//���ָ����RCC��־λ�������:�ܽŸ�λ
		{
      		;//printf("�ⲿ��λ������\r\n");
		}
    	//printf("����Ҫ���á�����\r\n");
		
		RTC_WaitForSynchro();										//�ȴ����һ�ζ�RTC�Ĵ�����д�������
		
		RTC_ITConfig(RTC_IT_SEC, ENABLE);							//ʹ��RTC���ж�

		RTC_WaitForLastTask();										//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	}
	
	rtc_get_time();
	
	RCC_ClearFlag();												//���RCC�ĸ�λ��־λ
}

void RTC_IRQHandler(void)
{							    
	if(RTC_GetITStatus(RTC_IT_SEC))			//�����ж�
	{							
		rtc_get_time();						//����ʱ�� 	 
	}
	if(RTC_GetITStatus(RTC_IT_ALR))			//�����ж�
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);	//�������ж�		  								 				
	}
	RTC_ClearITPendingBit(RTC_IT_SEC);		//�������������жϱ�־		  								 
	RTC_WaitForLastTask();					//�ȴ�RTC�Ĵ����������
}

u8 rtc_is_leap_year(u16 year)
{
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}

u8 rtc_update_time(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	u16 t;
	u32 seccount=0;
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)											//��������ݵ��������
		{
		if(rtc_is_leap_year(t))seccount+=31622400;						//�����������
		else seccount+=31536000;			  						//ƽ���������
		}
	smon-=1;
	for(t=0;t<smon;t++)	   											//��ǰ���·ݵ����������
		{
		seccount+=(u32)g_rtc_mon_table[t]*86400;						//�·����������
		if(rtc_is_leap_year(syear)&&t==1)seccount+=86400;				//����2�·�����һ���������	   
		}
	seccount+=(u32)(sday-1)*86400;								//��ǰ�����ڵ���������� 
	seccount+=(u32)hour*3600;									//Сʱ������
	seccount+=(u32)min*60;	 									//����������
	seccount+=sec;													//�������Ӽ���ȥ
											    
	RTC_WaitForLastTask();											//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	RTC_SetCounter(seccount);										//����RTC��������ֵ
	RTC_WaitForLastTask();											//�ȴ����һ�ζ�RTC�Ĵ�����д�������  	
	return 0;
}

u8 rtc_get_time(void)
{
	static u16 daycnt=0;
	u32 timecount=0; 
	u32 temp=0;
	u16 temp1=0;	  
	 
	timecount = RTC_GetCounter();  //��� RTC ������ֵ(������)											 
	temp=timecount/86400;   //�õ�����(��������Ӧ��)
	if(daycnt!=temp)//����һ����
	{	  
		daycnt=temp;
		temp1=1970;	//��1970�꿪ʼ
		while(temp>=365)
		{				 
			if(rtc_is_leap_year(temp1))//������
			{
				if(temp>=366)temp-=366;//�����������
				else {temp1++;break;}  
			}
			else temp-=365;	  //ƽ�� 
			temp1++;  
		}   
		g_rtc_date_time.year=temp1;//�õ����
		temp1=0;
		while(temp>=28)//������һ����
		{
			if(rtc_is_leap_year(g_rtc_date_time.year)&&temp1==1)//�����ǲ�������/2�·�
			{
				if(temp>=29)temp-=29;//�����������
				else break; 
			}
			else 
			{
				if(temp>=g_rtc_mon_table[temp1])temp-=g_rtc_mon_table[temp1];//ƽ��
				else break;
			}
			temp1++;  
		}
		g_rtc_date_time.month=temp1+1;//�õ��·�
		g_rtc_date_time.day=temp+1;  //�õ����� 
	}
	temp=timecount%86400;     //�õ�������   	   
	g_rtc_date_time.hour=temp/3600;     //Сʱ
	g_rtc_date_time.min=(temp%3600)/60; //����	
	g_rtc_date_time.sec=(temp%3600)%60; //����
	g_rtc_date_time.week=rtc_get_week(g_rtc_date_time.year,g_rtc_date_time.month,g_rtc_date_time.day);//��ȡ����   
	return 0;
}

u8 rtc_get_week(u16 year,u8 month,u8 day)
{
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 	 
	if (yearH>19)yearL+=100;// ���Ϊ21����,�������100 
  
	temp2=yearL+yearL/4;  	// ����������ֻ��1900��֮���
	temp2=temp2%7; 
	temp2=temp2+day+g_rtc_table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}

void rtc_set_time(void)
{
	rtc_update_time(
        g_rtc_date_time.year, 
        g_rtc_date_time.month, 
        g_rtc_date_time.day, 
        g_rtc_date_time.hour, 
        g_rtc_date_time.min, 
        g_rtc_date_time.sec);
}
