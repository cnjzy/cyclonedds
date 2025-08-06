#ifndef _OS_LOG_H
#define _OS_LOG_H

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
#include "base_types.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "os_stdio.h"
#include "os_types.h"
#include "os_thread.h"
#include "os_time.h"


#if defined(_WIN32)
#elif defined(__linux__)
#else
#define  ACore_SYSTEM
#endif



typedef enum os_log_level {
	OS_LOG_LEVEL_NO,
	OS_LOG_LEVEL_DEBUG,
	OS_LOG_LEVEL_INFO,
	OS_LOG_LEVEL_WARNING,
	OS_LOG_LEVEL_ERROR,
	OS_LOG_LEVEL_FATAL,
	OS_LOG_LEVEL_MAX
} os_log_level;
#define OS_LOG_OP_DEFAULT	(1u<<0)
#define OS_LOG_OP_STD		(1u<<1)
#define OS_LOG_OP_FILE		(1u<<2)
#define OS_LOG_OP_SIMPLE	(1u<<3)

typedef struct os_log_attr_s {
	os_log_level level;
	signed int output_type;
	signed char path[256];
	signed char log_file_name[256];
	/*单线程标志*/
	unsigned char single_thread_flag;
	unsigned long max_file_size; // 新增：日志文件的最大大小（单位：字节）
} os_log_attr_t;



#ifndef ACore_SYSTEM
static FILE* log_fp = 0;
static unsigned long s_max_log_file_size = 10 * 1024 * 1024; // 默认10MB
static signed char s_log_file_path[256] = "."; // 默认日志文件路径
static signed char s_log_file_name[256] = "common_log.txt"; // 默认日志文件路径
#endif

#define OS_LOG_MODEL_NAME_LENGTH 256
#define OS_LOG_MODULE_MAX_COUNT 100

typedef struct
{
	signed char os_log_model_name_default[OS_LOG_MODEL_NAME_LENGTH];
	os_log_level module_defualt_level;
	signed char path[256];
	signed char log_file_name[256];
	unsigned long max_file_size; // 新增：日志文件的最大大小（单位：字节）

}log_handle;


#define OS_LOG_DEBUG(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_DEBUG, OS_LOG_OP_DEFAULT, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))

#define OS_LOG_INFO(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_INFO, OS_LOG_OP_DEFAULT, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))

#define OS_LOG_WARNING(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_WARNING, OS_LOG_OP_DEFAULT, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))

#define OS_LOG_ERROR(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_ERROR, OS_LOG_OP_DEFAULT, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))

#define OS_LOG_FATAL(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_FATAL, OS_LOG_OP_DEFAULT, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))

#define OS_LOG_SIMPLE(log_handle_fp,level, ...) \
	(os_log_once(log_handle_fp,level, OS_LOG_OP_DEFAULT | OS_LOG_OP_SIMPLE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))

#define OS_LOG_SIMPLE_DEBUG(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_DEBUG, OS_LOG_OP_DEFAULT | OS_LOG_OP_SIMPLE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))
#define OS_LOG_SIMPLE_INFO(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_INFO, OS_LOG_OP_DEFAULT | OS_LOG_OP_SIMPLE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))
#define OS_LOG_SIMPLE_WARNING(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_WARNING, OS_LOG_OP_DEFAULT | OS_LOG_OP_SIMPLE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))
#define OS_LOG_SIMPLE_ERROR(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_ERROR, OS_LOG_OP_DEFAULT | OS_LOG_OP_SIMPLE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))
#define OS_LOG_SIMPLE_FATAL(log_handle_fp,...) \
	(os_log_once(log_handle_fp,OS_LOG_LEVEL_FATAL, OS_LOG_OP_DEFAULT | OS_LOG_OP_SIMPLE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__))

OS_API signed int os_log_init(const signed char* name, os_log_attr_t* attr, log_handle** log_handle_fp);

OS_API signed int os_log_once(log_handle* log_handle_fp, os_log_level level, signed int op_flag, const signed char* path, const signed char* func_name, signed int line, const signed char* format, ...);

/*不打印日志头，打印内容到控制台和日志文件内*/
OS_API signed int os_log_print(const signed char* format, ...);
#define OS_LOG_PRINT(...)  (os_log_print(__VA_ARGS__))

#ifndef ACore_SYSTEM
OS_API signed int os_log_close_file();
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus*/

#endif /* !_OS_LOG_H*/
