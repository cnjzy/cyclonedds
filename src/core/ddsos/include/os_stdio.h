#ifndef _OS_STDIO_H
#define _OS_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus*/

#include "os_types.h"
#include <stdio.h>

OS_API	signed int os_snprintf(signed char* buf, size_t size, const signed  char* format, ...);

OS_API	signed int os_vsnprintf(signed char* buf, size_t size, const signed  char* format, va_list args);

OS_API	signed int os_sleep(unsigned int milliseconds);

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif /* !_OS_STDIO_H*/
