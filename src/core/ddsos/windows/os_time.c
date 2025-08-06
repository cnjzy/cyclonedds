#include "os_time.h"
#include <windows.h>

OS_TIME os_getCurrentTime(void)
{
	OS_TIME os_time;
	memset(&os_time, 0, sizeof(OS_TIME));

	SYSTEMTIME st;
	memset(&st, 0, sizeof(SYSTEMTIME));


	GetLocalTime(&st);
	os_time.os_year = st.wYear;
	os_time.os_month = st.wMonth;
	os_time.os_day = st.wDay;
	os_time.os_hour = st.wHour;
	os_time.os_minute = st.wMinute;
	os_time.os_second = st.wSecond;
	os_time.os_millisecond = st.wMilliseconds;

	return os_time;
}

unsigned long long os_getCurrentEpochTime(void)
{
	SYSTEMTIME st;
	memset(&st, 0, sizeof(SYSTEMTIME));
	GetLocalTime(&st);
	unsigned long long ori_time = st.wMilliseconds;
	ori_time += st.wSecond * 1000;
	ori_time += st.wMinute * 60 * 1000;
	ori_time += st.wHour * 60 * 60 * 1000;
	ori_time += (st.wDay -1) * 24 * 60 * 60 * 1000;
	ori_time += (st.wMonth -1) * 30 * 24 * 60 * 60 * 1000;
	ori_time += (st.wYear - 1970) * 12 * 30 * 24 * 60 * 60 * 1000;
	return ori_time;
}
