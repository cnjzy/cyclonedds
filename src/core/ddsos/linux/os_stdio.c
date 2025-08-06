#include "os_stdio.h"
#include <stdarg.h>
#include <time.h>

signed int os_snprintf(signed char* buf, size_t size, const signed  char* format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = os_vsnprintf(buf, size, format, args);
	va_end(args);
	return ret;
}

signed int os_vsnprintf(signed char* buf, size_t size, const signed  char* format, va_list args)
{
	return vsnprintf(buf, size, format, args);
}

signed int os_sleep(unsigned int milliseconds)
{
	struct timespec t_sleep;
	t_sleep.tv_sec = milliseconds / 1000;
	t_sleep.tv_nsec = (milliseconds - t_sleep.tv_sec * 1000) * 1000000;
	return nanosleep(&t_sleep, NULL);
}
