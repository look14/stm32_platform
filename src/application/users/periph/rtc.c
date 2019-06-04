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
																	//使能PWR和BKP外设时钟   		
	PWR_BackupAccessCmd(ENABLE);									//使能RTC和后备寄存器访问 
	
	if(BKP_ReadBackupRegister(BKP_DR1)!=0x5555)						//从指定的后备寄存器中读出数据，判断是否为第一次配置
	{	
      	//printf("时钟配置。。。\r\n");																
		BKP_DeInit();												//将外设BKP的全部寄存器重设为缺省值 	
		RCC_LSEConfig(RCC_LSE_ON);									//使能外部低速时钟 32.768KHz
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)			//检查指定的RCC标志位设置与否,等待低速晶振就绪
  		{}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);						//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
		RCC_RTCCLKCmd(ENABLE);										//使能RTC时钟  
		RTC_WaitForSynchro();										//等待RTC寄存器(RTC_CNT,RTC_ALR和RTC_PRL)与RTC APB时钟同步
		RTC_WaitForLastTask();										//等待最近一次对RTC寄存器的写操作完成
		RTC_ITConfig(RTC_IT_SEC, ENABLE);							//使能RTC秒中断
		RTC_WaitForLastTask();										//等待最近一次对RTC寄存器的写操作完成
		RTC_SetPrescaler(32767); 									//设置RTC预分频的值  RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
		RTC_WaitForLastTask();										//等待最近一次对RTC寄存器的写操作完成
		rtc_set_time();												//时间设置	
    	BKP_WriteBackupRegister(BKP_DR1, 0x5555);					//向指定的后备寄存器中写入用户程序数据0X5555做判断标志										
	}																 	
	else															//不是第一次配置 继续计时
	{
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)			//检查指定的RCC标志位设置与否:POR/PDR复位
		{
      		;//printf("上电复位。。。\r\n");
		}											
		else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)		//检查指定的RCC标志位设置与否:管脚复位
		{
      		;//printf("外部复位。。。\r\n");
		}
    	//printf("不需要配置。。。\r\n");
		
		RTC_WaitForSynchro();										//等待最近一次对RTC寄存器的写操作完成
		
		RTC_ITConfig(RTC_IT_SEC, ENABLE);							//使能RTC秒中断

		RTC_WaitForLastTask();										//等待最近一次对RTC寄存器的写操作完成
	}
	
	rtc_get_time();
	
	RCC_ClearFlag();												//清除RCC的复位标志位
}

void RTC_IRQHandler(void)
{							    
	if(RTC_GetITStatus(RTC_IT_SEC))			//秒钟中断
	{							
		rtc_get_time();						//更新时间 	 
	}
	if(RTC_GetITStatus(RTC_IT_ALR))			//闹钟中断
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);	//清闹钟中断		  								 				
	}
	RTC_ClearITPendingBit(RTC_IT_SEC);		//清除溢出，秒钟中断标志		  								 
	RTC_WaitForLastTask();					//等待RTC寄存器操作完成
}

u8 rtc_is_leap_year(u16 year)
{
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}

u8 rtc_update_time(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	u16 t;
	u32 seccount=0;
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)											//把所有年份的秒钟相加
		{
		if(rtc_is_leap_year(t))seccount+=31622400;						//闰年的秒钟数
		else seccount+=31536000;			  						//平年的秒钟数
		}
	smon-=1;
	for(t=0;t<smon;t++)	   											//把前面月份的秒钟数相加
		{
		seccount+=(u32)g_rtc_mon_table[t]*86400;						//月份秒钟数相加
		if(rtc_is_leap_year(syear)&&t==1)seccount+=86400;				//闰年2月份增加一天的秒钟数	   
		}
	seccount+=(u32)(sday-1)*86400;								//把前面日期的秒钟数相加 
	seccount+=(u32)hour*3600;									//小时秒钟数
	seccount+=(u32)min*60;	 									//分钟秒钟数
	seccount+=sec;													//最后的秒钟加上去
											    
	RTC_WaitForLastTask();											//等待最近一次对RTC寄存器的写操作完成
	RTC_SetCounter(seccount);										//设置RTC计数器的值
	RTC_WaitForLastTask();											//等待最近一次对RTC寄存器的写操作完成  	
	return 0;
}

u8 rtc_get_time(void)
{
	static u16 daycnt=0;
	u32 timecount=0; 
	u32 temp=0;
	u16 temp1=0;	  
	 
	timecount = RTC_GetCounter();  //获得 RTC 计数器值(秒钟数)											 
	temp=timecount/86400;   //得到天数(秒钟数对应的)
	if(daycnt!=temp)//超过一天了
	{	  
		daycnt=temp;
		temp1=1970;	//从1970年开始
		while(temp>=365)
		{				 
			if(rtc_is_leap_year(temp1))//是闰年
			{
				if(temp>=366)temp-=366;//闰年的秒钟数
				else {temp1++;break;}  
			}
			else temp-=365;	  //平年 
			temp1++;  
		}   
		g_rtc_date_time.year=temp1;//得到年份
		temp1=0;
		while(temp>=28)//超过了一个月
		{
			if(rtc_is_leap_year(g_rtc_date_time.year)&&temp1==1)//当年是不是闰年/2月份
			{
				if(temp>=29)temp-=29;//闰年的秒钟数
				else break; 
			}
			else 
			{
				if(temp>=g_rtc_mon_table[temp1])temp-=g_rtc_mon_table[temp1];//平年
				else break;
			}
			temp1++;  
		}
		g_rtc_date_time.month=temp1+1;//得到月份
		g_rtc_date_time.day=temp+1;  //得到日期 
	}
	temp=timecount%86400;     //得到秒钟数   	   
	g_rtc_date_time.hour=temp/3600;     //小时
	g_rtc_date_time.min=(temp%3600)/60; //分钟	
	g_rtc_date_time.sec=(temp%3600)%60; //秒钟
	g_rtc_date_time.week=rtc_get_week(g_rtc_date_time.year,g_rtc_date_time.month,g_rtc_date_time.day);//获取星期   
	return 0;
}

u8 rtc_get_week(u16 year,u8 month,u8 day)
{
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 	 
	if (yearH>19)yearL+=100;// 如果为21世纪,年份数加100 
  
	temp2=yearL+yearL/4;  	// 所过闰年数只算1900年之后的
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
