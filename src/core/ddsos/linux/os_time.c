#include "os_time.h"
#include <sys/timeb.h>
//#include <chrono>

OS_TIME os_getCurrentTime()
{
	OS_TIME os_time;


	struct timeb tp_cur;
	ftime(&tp_cur);

	struct tm btm;
	localtime_r(&tp_cur.time,&btm);



	os_time.os_year = (unsigned short) (btm.tm_year + 1900);
	os_time.os_month = (unsigned short)(btm.tm_mon+1);
	os_time.os_day = (unsigned short)btm.tm_mday;
	os_time.os_hour = (unsigned short)btm.tm_hour;
	os_time.os_minute = (unsigned short)btm.tm_min;
	os_time.os_second = (unsigned short)btm.tm_sec;
	os_time.os_millisecond = (unsigned short)tp_cur.millitm;

	return os_time;
}

unsigned long long os_getCurrentEpochTime(void)
{
	struct timeb tp_cur;
	ftime(&tp_cur);

	return tp_cur.time * 1000 + tp_cur.millitm;
}