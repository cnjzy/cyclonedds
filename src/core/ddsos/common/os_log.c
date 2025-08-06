#include "os_log.h"

/*
#ifndef ACore_SYSTEM
static FILE* log_fp = 0;
static unsigned long s_max_log_file_size = 10 * 1024 * 1024; // Ĭ��10MB
static signed char s_log_file_path[256] = "."; // Ĭ����־�ļ�·��
static signed char s_log_file_name[256] = "common_log.txt"; // Ĭ����־�ļ�·��
#endif
*/
#define OS_LOG_APP_NAME_LENGTH 256
#define OS_LOG_FILE_NAME_LENGTH 256
#define OS_LOG_FUNC_NAME_LENGTH 256
#define OS_LOG_CONTENT_LENGTH 1024
#define OS_LOG_QUEUE_CONTENT_MAX_COUNT 500

typedef struct os_log_item_s {
	unsigned long long time_stamp;

	signed char app_name[OS_LOG_APP_NAME_LENGTH];		/*从 argv 获取，暂时不处理*/
	signed char model_name[OS_LOG_MODEL_NAME_LENGTH];

	UINT32 proc_id;			/* 暂时不处理*/
	ULONG thread_id;

	signed char filename[OS_LOG_FILE_NAME_LENGTH];		/*太长，暂时不打印*/
	signed char filepath[OS_LOG_FILE_NAME_LENGTH];		/*太长，暂时不打印*/
	signed char func_name[OS_LOG_FUNC_NAME_LENGTH];
	signed int line;

	os_log_level level;

	signed int op_flag;
	int is_simple;
	signed char content[OS_LOG_CONTENT_LENGTH + 1 + 1]; /* 增加换行， 增加\0 */
} os_log_item_t;

static signed int os_log_op_default = OS_LOG_OP_STD | OS_LOG_OP_FILE;
static signed char os_log_app_name_default[OS_LOG_APP_NAME_LENGTH] = "os_log";

static signed int os_log_item_init(log_handle* log_handle_fp,os_log_item_t* item, os_log_level level, const signed char* path, const signed char* func_name, signed int line, const signed char* format, va_list args);
static signed int os_log_item_content( signed char* content, const signed char* format, va_list args);
static signed int os_log_item_std_out(os_log_item_t* item);
static signed int os_log_item_file_out(os_log_item_t* item);
static signed int os_log_create_out_file(char* filePath, char* fileName);

static log_handle log_handle_attr[OS_LOG_MODULE_MAX_COUNT] = {0};
static signed int log_handle_count = 0;

/*多线程打印标志*/
static unsigned char mul_thread_flag = 0;
static int noLogFlg = 0;

typedef struct {
	os_log_item_t output_items[OS_LOG_QUEUE_CONTENT_MAX_COUNT];
	signed int count;
}log_output_queue;

static log_output_queue log_queue;

static void insert_output_item(os_log_item_t item)
{

	assert(log_queue.count < OS_LOG_QUEUE_CONTENT_MAX_COUNT);
	log_queue.output_items[log_queue.count] = item;
	if (log_queue.count < OS_LOG_QUEUE_CONTENT_MAX_COUNT-1)
	{	
		log_queue.count++;
	}
}


static signed int output_item(os_log_item_t item)
{
	signed int ret = 0;

	signed int flag = 0;
	if (item.op_flag & (signed int)OS_LOG_OP_DEFAULT) {
		flag = os_log_op_default | item.op_flag;
	}
	/* 日志打印至标准输出 */
	if (flag & (signed int)OS_LOG_OP_STD) {
		ret = os_log_item_std_out(&item);
	}
	/* 日志打印至文件*/
	if (flag & (signed int)OS_LOG_OP_FILE) {
		ret = os_log_item_file_out(&item);
	}
	return ret;
}

static signed int clear_output_item_queue()
{
	log_output_queue log_queue_tmp = { 0 };
	memcpy(&log_queue_tmp, &log_queue, sizeof(log_queue));
	log_queue.count = 0;
	signed int ret = 0;
	for (signed int i = 0; i < log_queue_tmp.count; i++)
	{
		output_item(log_queue_tmp.output_items[i]);
	}

	return ret;
}




static unsigned char first_init_log_flag=0;
void* log_thread_func(void)
{
	while (1)
	{
		clear_output_item_queue();
		os_sleep(5);
	}
}

signed int os_log_init( const signed char* name, os_log_attr_t* attr,log_handle** log_handle_fp)
{

	if (OS_LOG_LEVEL_NO == attr->level)
	{
		noLogFlg = 1;
		return 0;
	}

	noLogFlg = 0;


	if (0 == attr->single_thread_flag)
	{
		/*多线程标志置为1*/
		mul_thread_flag = 1;
	}

	if (0 == first_init_log_flag && mul_thread_flag)
	{

		os_thread_t os_log_thread;
		os_thread_attr_t attr_log;
		os_thread_attr_init(&attr_log);
		char name[] = "os_log_thread";

		os_thread_create(&os_log_thread, name, &attr_log, (os_thread_route)log_thread_func, 0);

		first_init_log_flag = 1;
	}

	if (name) {
		if ('\0' != name[0]) {

			if (OS_LOG_MODULE_MAX_COUNT > log_handle_count)
			{
				*log_handle_fp = &log_handle_attr[log_handle_count];
				memset((*log_handle_fp)->os_log_model_name_default, 0, sizeof(log_handle));
				memcpy((*log_handle_fp)->os_log_model_name_default, name, strlen(name));
				(*log_handle_fp)->os_log_model_name_default[strlen(name)] = '\0';

				log_handle_count++;
			}
			else
			{
				return RET_NO_ERR;
			}

		}
	}
	if (attr == NULL) {
		(*log_handle_fp)->module_defualt_level = OS_LOG_LEVEL_WARNING;
		return RET_NO_ERR;
	}
	if (attr->level < OS_LOG_LEVEL_MAX) {
		(*log_handle_fp)->module_defualt_level = attr->level;
		strncpy((*log_handle_fp)->path, attr->path, sizeof((*log_handle_fp)->path) - 1);
		(*log_handle_fp)->path[sizeof((*log_handle_fp)->path) - 1] = '\0'; // Ensure null-termination 
		strncpy((*log_handle_fp)->log_file_name, attr->log_file_name, sizeof((*log_handle_fp)->log_file_name) - 1);
		(*log_handle_fp)->path[sizeof((*log_handle_fp)->log_file_name) - 1] = '\0'; // Ensure null-termination 
		(*log_handle_fp)->max_file_size = attr->max_file_size;
	}


	/*日志文件*/

	OS_TIME os_time = os_getCurrentTime();

	signed char time[64];

	if ((0 == os_time.os_year) || (0 == os_time.os_month) || (0 == os_time.os_day))
	{
		os_snprintf(time, 64, "%02d_%02d_%02d",
			os_time.os_hour,
			os_time.os_minute,
			os_time.os_second);
	}
	else
	{
		os_snprintf(time, 64, "%02d_%02d_%02d_%02d_%02d_%02d",
			os_time.os_year%100,
			os_time.os_month,
			os_time.os_day,
			os_time.os_hour,
			os_time.os_minute,
			os_time.os_second);
	}

#ifndef ACore_SYSTEM
	/*打开文件描述符*/
	signed char log_file_name[512];

	/*当传入的日志参数为空指针或传入的日志参数内有日志文件名*/
	if (attr)
	{
		s_max_log_file_size = attr->max_file_size > 0 ? attr->max_file_size : 10 * 1024 * 1024; // 默认10MB
		strncpy(s_log_file_path, attr->path, sizeof(s_log_file_path) - 1);
		(*log_handle_fp)->path[sizeof(s_log_file_path) - 1] = '\0'; // Ensure null-termination 
		strncpy(s_log_file_name, attr->log_file_name, sizeof(s_log_file_name) - 1);
		(*log_handle_fp)->path[sizeof(s_log_file_name) - 1] = '\0'; // Ensure null-termination 
		os_log_create_out_file(attr->path, attr->log_file_name);
	}
	else
	{
		s_max_log_file_size = 10 * 1024 * 1024; // 默认10MB
		os_log_create_out_file(s_log_file_path, s_log_file_name);
	}
	/*
	log_fp = fopen(log_file_name, "a");
	if (log_fp <= 0)
	{
		log_fp = 0;
		return RET_ERR;
	}
	*/
#endif

	return RET_NO_ERR;
}

signed int os_log_once(log_handle* log_handle_fp,os_log_level level, signed int op_flag, const signed char* path, const signed char* func_name, signed int line, const signed char* format, ...)
{
	if (noLogFlg)
	{
		return 0;
	}

	if (level < log_handle_fp->module_defualt_level || level >= OS_LOG_LEVEL_MAX) {
		return 0;
	}

	os_log_item_t item;
	memset(&item, 0, sizeof(os_log_item_t));

	item.op_flag = op_flag;
	if (op_flag & OS_LOG_OP_SIMPLE) {
		item.is_simple = 1;
	}

	signed int ret = 0;

	va_list args;
	va_start(args, format);
	ret = os_log_item_init(log_handle_fp, &item, level, path, func_name, line, format, args);
	va_end(args);


	/*当为多线程时将内容入队列*/
	if (mul_thread_flag)
	{	
		/*插入item到queue*/
		insert_output_item(item);
	}
	else/*单线程时，*/
	{
		output_item(item);
	}


	return ret;
}

OS_API signed int os_log_print(const signed char* format, ...)
{
	signed char content[OS_LOG_CONTENT_LENGTH];
	va_list args;
	va_start(args, format);
	signed int ret = os_vsnprintf(content, OS_LOG_CONTENT_LENGTH, format, args);
	va_end(args);

	if (ret < 0) {
		printf("[ERROR] %s: vsnprintf fail\n", __FUNCTION__);
		return RET_ERR;
	}
	content[ret] = '\0';


	printf("%s", content);

#ifndef ACore_SYSTEM
	if (0 != log_fp) 
	{
		fprintf(log_fp, content);
		fflush(log_fp);  /*刷新输出缓冲区，确保数据写入文件 */
	}
#endif

	return RET_NO_ERR;

}

#ifndef ACore_SYSTEM
OS_API signed int os_log_close_file()
{
	if (log_fp != NULL) {
		fclose(log_fp);
		log_fp = NULL;
		return RET_NO_ERR;
	}

	return RET_ERR;
}
#endif

signed int os_log_item_init(log_handle* log_handle_fp, os_log_item_t* item, os_log_level level, const signed char* path, const signed char* func_name, signed int line, const signed char* format, va_list args)
{
	item->time_stamp = 0;

	memcpy(item->app_name, os_log_app_name_default, sizeof(os_log_app_name_default));
	item->app_name[sizeof(os_log_app_name_default)] = '\0';

	memcpy(item->model_name, log_handle_fp->os_log_model_name_default, sizeof(log_handle_fp->os_log_model_name_default));
	item->model_name[sizeof(log_handle_fp->os_log_model_name_default)] = '\0';

	item->thread_id = os_thread_to_integer(os_thread_self());
	memcpy(item->filename, path, strlen(log_handle_fp->log_file_name));
	memcpy(item->func_name, func_name, strlen(func_name));
	memcpy(item->filepath, log_handle_fp->path, strlen(log_handle_fp->path));
	item->line = line;

	item->level = level;

	os_log_item_content(item->content, format, args);

	return 0;
}

signed int os_log_item_content(signed char* content, const signed char* format, va_list args)
{
	signed int ret = os_vsnprintf(content, OS_LOG_CONTENT_LENGTH, format, args);
	if (ret < 0) {
		printf("[ERROR] %s: vsnprintf fail\n", __FUNCTION__);
	}
	content[ret++] = '\n';	
	content[ret] = '\0';

	return ret;
}

static signed int os_log_item_struct2buffer(const os_log_item_t* item, signed char* buf, unsigned int size)
{
	if (item->is_simple) {
		os_snprintf(buf, sizeof(buf), "%s|%s", item->model_name, item->content);
	}
	else {
		signed char level[64];
		const signed char* log_level[OS_LOG_LEVEL_MAX] = { "", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" };

		for (signed int n = 0; n < OS_LOG_LEVEL_MAX; ++n) {
			if (n == item->level) {
				memcpy(level, log_level[n], strlen(log_level[n]));
				level[strlen(log_level[n])] = '\0';
			}
		}

		OS_TIME os_time = os_getCurrentTime();
		signed char time[64];

		if ((0 == os_time.os_year) || (0 == os_time.os_month) || (0 == os_time.os_day))
		{
			os_snprintf(time, 64, "%02d:%02d:%02d:%03d",
				os_time.os_hour,
				os_time.os_minute,
				os_time.os_second,
				os_time.os_millisecond);
		}
		else
		{
			os_snprintf(time, 64, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
				os_time.os_year,
				os_time.os_month,
				os_time.os_day,
				os_time.os_hour,
				os_time.os_minute,
				os_time.os_second,
				os_time.os_millisecond);
		}

		os_snprintf(buf, OS_LOG_CONTENT_LENGTH * 2,
			/* 时间戳|模块名|线程ID|函数名|所在行|日志等级|内容*/
			"%s|%s|%lu|%s|%d|%s|%s",
			item->model_name, time, item->thread_id, item->func_name, item->line, level, item->content);
	}
	return 0;
}

signed int os_log_item_std_out(os_log_item_t* item)
{
	signed char buf[OS_LOG_CONTENT_LENGTH * 2];
	memset(buf, 0, sizeof(buf));

	if (item->is_simple) {
		os_snprintf(buf, sizeof(buf), "%s|%s", item->model_name, item->content);
	}
	else {
		os_log_item_struct2buffer(item, buf, sizeof(buf));
	}

	printf("%s", buf);

	return 0;
}

static signed int os_log_item_file_out(os_log_item_t* item)
{
#ifndef ACore_SYSTEM


	signed char buf[OS_LOG_CONTENT_LENGTH * 2];
	memset(buf, 0, sizeof(buf));

	if (item->is_simple) {
		os_snprintf(buf, sizeof(buf), "%s|%s", item->model_name, item->content);
	}
	else {
		os_log_item_struct2buffer(item, buf, sizeof(buf));
	}

	os_log_create_out_file(item->filepath, item->filename);

	// 写入日志文件
	fprintf(log_fp, buf);
	fflush(log_fp);  /*刷新输出缓冲区，确保数据写入文件 */
#endif
	return 0;
}

signed int os_log_create_out_file(char* filePath, char* fileName)
{
	if (!filePath || '\0' == filePath[0])
	{
		filePath = ".";
	}
	if (!fileName || '\0' == fileName[0])
	{
		fileName = "log";
	}

	OS_TIME os_time = os_getCurrentTime();

	// 检查文件是否存在
	if (log_fp > 0) {
		// 文件存在，检查文件大小
		fseek(log_fp, 0, SEEK_END);
		long file_size = ftell(log_fp);
		if (file_size >= s_max_log_file_size) {
			// 文件大小超过限制，创建新文件
			char newFileName[512] = { 0 };

			snprintf(newFileName, sizeof(newFileName), "%s/%s_%04d%02d%02d%02d%02d%02d.txt",
				filePath, fileName,
				os_time.os_year, os_time.os_month, os_time.os_day,
				os_time.os_hour, os_time.os_minute, os_time.os_second);

			// 打开新文件
			log_fp = fopen(newFileName, "a");
			if (log_fp <= 0) {
				printf("[ERROR] Failed to create new log file: %s\n", newFileName);
				return RET_ERR;
			}
			printf("[INFO] Created new log file: %s\n", newFileName);
		}
	}
	else 
	{
		// 定义文件路径缓冲区
		char fullFilePath[512] = { 0 };
		snprintf(fullFilePath, sizeof(fullFilePath), "%s/%s_%04d%02d%02d%02d%02d%02d.txt",
			filePath, fileName,
			os_time.os_year, os_time.os_month, os_time.os_day,
			os_time.os_hour, os_time.os_minute, os_time.os_second);

		// 文件不存在，创建新文件
		log_fp = fopen(fullFilePath, "a");
		if (log_fp <= 0) {
			printf("[ERROR] Failed to create log file: %s\n", fullFilePath);
			return RET_ERR;
		}
		printf("[INFO] Created new log file: %s\n", fullFilePath);
	}

	return RET_NO_ERR;
}

#undef OS_LOG_DEV_NAME_LENGTH
#undef OS_LOG_APP_NAME_LENGTH
#undef OS_LOG_FILE_NAME_LENGTH
#undef OS_LOG_FUNC_NAME_LENGTH
#undef OS_LOG_CONTENT_LENGTH
