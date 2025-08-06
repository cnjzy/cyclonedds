/*
 * os_stdio.c
 *
 *  Created on: 2024Äê6ÔÂ6ÈÕ
 *      Author: Admin
 */

#include "os_stdio.h"
#include <stdarg.h>
#include <os/ac653ApexTypes.h>
#include <os/pos/apex/apexProcess.h>

OS_API	signed int os_snprintf(signed char* buf, size_t size, const signed  char* format, ...)
{
	va_list args;
	va_start(args, format);
	signed int ret = os_vsnprintf(buf, size, format, args);
	va_end(args);
	return ret;
}

OS_API	signed int os_vsnprintf(signed char* buf, size_t size, const signed char* format, va_list args)
{
	
	return vsnprintf((char*)buf, size, (const char*)format, args);
	
}

signed int os_sleep(unsigned int milliseconds)
{
	RETURN_CODE_TYPE retCode = NO_ERROR;
	SUSPEND_SELF(milliseconds * 1000000,&retCode);
	return 0;
}
