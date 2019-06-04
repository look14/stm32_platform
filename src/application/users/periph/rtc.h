#ifndef __RTC_H
#define __RTC_H

#include "typedefs.h"

typedef struct __rtc_date_time {
	u8 hour;
	u8 min;
	u8 sec;
	u16 year;
	u8 month;
	u8 day;
	u8 week;
}rtc_date_time;

extern volatile rtc_date_time g_rtc_date_time;

void rtc_init(void);
u8 rtc_is_leap_year(u16 year);
u8 rtc_update_time(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);
u8 rtc_get_time(void);
u8 rtc_get_week(u16 year,u8 month,u8 day);
void rtc_set_time(void);

#endif
