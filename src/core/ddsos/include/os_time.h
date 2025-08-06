#ifndef _OS_TIME_H_
#define _OS_TIME_H_
#include "os_defs.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "os_types.h"


	typedef struct
	{
		unsigned short os_year;
		unsigned short os_month;
		unsigned short os_day;
		unsigned short os_hour;
		unsigned short os_minute;
		unsigned short os_second;
		unsigned short os_millisecond;
	}OS_TIME;

	OS_TIME os_getCurrentTime(void);

	/*返回以1970.1.1 0：0：0 为开始的毫秒*/
	unsigned long long os_getCurrentEpochTime(void);


#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif /* !_OS_TIME_H_*/
