#include "os_stdio.h"
#include <stdarg.h>
#include <Windows.h>

signed int os_snprintf(signed char* buf, size_t size, const signed char* format, ...)
{
	va_list args;
	memset(&args, 0, sizeof(va_list));

	va_start(args, format);
	signed  int ret = os_vsnprintf(buf, size, format, args);
	va_end(args);
	return ret;
}


signed int os_vsnprintf(signed char* buf, size_t size, const signed char* format, va_list args)
{
	return _vsnprintf_s(buf, size, size, format, args);
}

signed int os_sleep(unsigned int milliseconds)
{
	Sleep(milliseconds);
	return 0;
}

