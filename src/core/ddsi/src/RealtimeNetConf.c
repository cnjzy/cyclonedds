//#include "dds/ddsi/RealtimeNetConf.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dds/ddsi/RealtimeNetConf.h"



#ifdef _WIN32
#define str_to_int64(str) _atoi64(str)
#else
#define str_to_int64(str) strtoll(str, NULL, 10)  // 十进制转换:ml-citation{ref="7" data="citationList"}
#endif

size_t ac_regular_file_size(const char *filename)
{
	if (filename)
	{
#if _WIN32
		struct _stat stat_info;
		if (_stat(filename, &stat_info) == 0)
			if (stat_info.st_mode & _S_IFREG)
				return (size_t)stat_info.st_size;
#else
		struct stat stat_info;
		if (stat(filename, &stat_info) == 0)
			if (S_ISREG(stat_info.st_mode))
				return (size_t)stat_info.st_size;
#endif
	}
	return 0;
}

/**
 * @brief 申请配置信息内存
 *
 * @param
 * @param
 * @return 指向配置信息的结构体指针
 *
 * 为配置信息结构体申请内存，并初始化为0
 */
RealtimeNetConf *  RealtimeNetReadConf_malloc()
{
	//申请内存
	RealtimeNetConf * conf = malloc(sizeof(RealtimeNetConf));

	//初始化为0
	memset(conf, 0, sizeof(RealtimeNetConf));

	return conf;
}

/**
 * @brief 释放配置信息内存
 *
 * @param pConf 配置信息指针
 * @param
 * @return 无
 *
 * 为配置信息结构体释放内存，内部的指针也释放内存
 */
void  RealtimeNetReadConf_free(RealtimeNetConf * pConf)
{
	//释放partition内存
	if (NULL != pConf->qos.partition) {
		free(pConf->qos.partition);
		pConf->qos.partition = NULL;
	}

	//释放整体内存
	free(pConf);
}

/**
 * @brief 读取配置
 *
 * @param path 配置文件的全路径
 * @param pConf 读取到的配置信息
 * @return int 成功返回1，失败返回0
 *
 * 根据偏移量在哈希表中查找对应的Topic。
 */
int  RealtimeNetReadConf(char * fname, RealtimeNetConf * pConf)
{
	char *buf = NULL;
	//把文件数据加载到缓冲区
	RealtimeNetReadConf_fileToBuf(fname, &buf);
	//printf("filebuf:%s\n", buf);
	//printf("\n");
	//初始化配置信息的内容，所有值均赋值默认值
	RealtimeNetReadConf_initConf(pConf);

	//把缓冲区的数据转化成配置信息
	RealtimeNetReadConf_bufToConf(buf, pConf);

	free(buf);

	//打印配置信息
	RealtimeNetReadConf_printConf(pConf);

	return 1;
}

/**
 * @brief 把配置文件中的内容加载到缓冲区
 *
 * @param fname 配置文件全路径
 * @param buf 保存配置文件内容的缓冲区
 * @return 函数调用结果 1-成功 0-失败
 *
 * 读取配置文件中的内容，申请内存，把文件的内容拷贝到内存
 */
int  RealtimeNetReadConf_fileToBuf(char * fname, char **buf)
{
	size_t sz;
	size_t fileLength;
	FILE *fp = NULL;
	char *document = NULL;
	/* Get size if it is a accessible regular file (no dir or link). */
	sz = ac_regular_file_size(fname);
	if (sz > 0)
	{
		/* Open the actual file. */
		fp = fopen(fname, "r");
		if (fp)
		{
			/* Read the content. */
			document = malloc(sz + 1);
			memset(document, 0, sz + 1);
			fileLength = fread(document, 1, sz, fp);
			if (fileLength == 0)
			{
				free(document);
			}
			else
			{
				document[fileLength] = '\0';
				*buf = document;
			}
			(void)fclose(fp);
		}
	}
	return 1;
}

/**
 * @brief 初始化配置信息
 *
 * @param pConf 配置信息结构体
 * @param
 * @return
 *
 * 给配置信息赋默认值
 */
void  RealtimeNetReadConf_initConf(RealtimeNetConf * pConf)
{

	//license
	//strncpy(pConf->lice.licencePath, ".", sizeof(pConf->lice.licencePath));

	//comm
	pConf->comPro.type = 0; //udp

	//Qos
	//set durability
	pConf->qos.durability_kind = DDS_DURABILITY_VOLATILE;
	//set presentation
	pConf->qos.presentation.access_scope = DDS_PRESENTATION_INSTANCE;
	pConf->qos.presentation.coherent_access = false;
	pConf->qos.presentation.ordered_access = false;
	//set deadline
	pConf->qos.deadline = 0;
	//set ownership
	pConf->qos.ownership_kind = DDS_OWNERSHIP_SHARED;
	//set liveliness
	pConf->qos.liveliness.kind = DDS_LIVELINESS_AUTOMATIC;
	pConf->qos.liveliness.lease_duration = DDS_INFINITY;
	//set time_based_filter
	pConf->qos.minimum_separation = 0;
	//set Reliability
	pConf->qos.reliability.kind = DDS_RELIABILITY_BEST_EFFORT;
	pConf->qos.reliability.max_blocking_time = DDS_INFINITY;
	//set destination_order
	pConf->qos.destination_order.kind = DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP;
	//set History
	pConf->qos.history.kind = DDS_HISTORY_KEEP_LAST;
	pConf->qos.history.depth = 1;
	//set default value
	pConf->qos.resource_limits.max_samples = 10000;
	pConf->qos.resource_limits.max_instances = DDS_LENGTH_UNLIMITED;
	pConf->qos.resource_limits.max_samples_per_instance = DDS_LENGTH_UNLIMITED;

	//log
	//set logLevel
	pConf->log.logLevel = 0;
	//set logPath
	//strncpy(pConf->log.logPath, ".", sizeof(pConf->log.logPath) - 1);
	//maxFileSize
	pConf->log.maxFileSize = 1024;
}

/**
 * @brief 设置QOS信息
 *
 * @param qos dds的qos结构体
 * @param myQos 业务定义的qos结构体
 * @return
 *
 * 把业务定义的qos信息赋值给dds的qos结构体
 */
void  RealtimeNetReadConf_setQos(dds_qos_t * qos, Qos * myQos)
{
	// todo 20250715 添加有效性
	/* 持久性策略 */
	if (myQos->durability_kind_valid)
	{
		dds_qset_durability(qos, myQos->durability_kind);
	}

	/* 呈现策略 控制数据的呈现方式，包括访问范围、是否支持一致访问和有序访问等 */
	if (myQos->presentation_valid)
	{
		dds_qset_presentation(qos, myQos->presentation.access_scope, myQos->presentation.coherent_access, myQos->presentation.ordered_access);
	}

	/* 截止时间策略 */
	if (myQos->deadline_valid)
	{
		if (myQos->deadline)
		{
			dds_qset_deadline(qos, myQos->deadline);
		}
	}

	/* 所有权策略 决定了数据的所有权归属，DDS_OWNERSHIP_SHARED 表示数据可以被多个写入者共享，而 DDS_OWNERSHIP_EXCLUSIVE 表示数据只能由一个写入者拥有*/
	if (myQos->ownership_kind_valid)
	{
		dds_qset_ownership(qos, myQos->ownership_kind);
	}
	/* 活跃度策略 用于检测实体是否仍然活跃，DDS_LIVELINESS_AUTOMATIC 表示系统自动检测，DDS_LIVELINESS_MANUAL_BY_PARTICIPANT 表示由参与者手动检测，DDS_LIVELINESS_MANUAL_BY_TOPIC 表示由主题手动检测 */
	if (myQos->liveliness_valid)
	{
		dds_qset_liveliness(qos, myQos->liveliness.kind, myQos->liveliness.lease_duration);
	}
	/* 基于时间的过滤策略 指定了数据接收的最小时间间隔 */
	if (myQos->time_based_filter_valid)
	{
		dds_qset_time_based_filter(qos, myQos->minimum_separation);
	}
	/* 分区策略 将实体划分到不同的分区中，只有同一分区内的实体才能进行通信 */
	if (myQos->partition_valid)
	{
		dds_qset_partition1(qos, myQos->partition);
	}
	/* 可靠性策略 */
	if (myQos->reliability_valid)
	{
		dds_qset_reliability(qos, myQos->reliability.kind, myQos->reliability.max_blocking_time);
	}
	/* 目标顺序策略 */
	if (myQos->destination_order_valid)
	{
		dds_qset_destination_order(qos, myQos->destination_order.kind);
	}
	/* 历史策略 */
	if (myQos->history_valid)
	{
		dds_qset_history(qos, myQos->history.kind, myQos->history.depth);
	}
	/* 资源限制策略 */
	if (myQos->resource_limits_valid)
	{
		dds_qset_resource_limits(qos, myQos->resource_limits.max_instances, myQos->resource_limits.max_samples, myQos->resource_limits.max_samples_per_instance);
	}
}

/**
 * @brief 打印配置信息结构体
 *
 * @param pConf 配置信息结构体
 * @param
 * @return
 *
 * 打印配置信息结构体
 */
void  RealtimeNetReadConf_printConf(RealtimeNetConf * pConf) 
{
	printf("-------------------printconf-------------------------------------\n");
	//license
	printf("licencePath[%s]\n", pConf->lice.licencePath);

	//comm
	printf("pConf->comPro.type[%d]\n", pConf->comPro.type);

	//Qos
	//set durability
	printf("pConf->qos.durability_kind[%d]\n", pConf->qos.durability_kind);
	//set presentation
	printf("pConf->qos.presentation.access_scope[%d]\n", pConf->qos.presentation.access_scope);
	printf("pConf->qos.presentation.coherent_access[%d]\n", pConf->qos.presentation.coherent_access);
	printf("pConf->qos.presentation.ordered_access[%d]\n", pConf->qos.presentation.ordered_access);
	//set deadline
	printf("pConf->qos.deadline[%lld]\n", pConf->qos.deadline);
	//set ownership
	printf("pConf->qos.ownership_kind[%d]\n", pConf->qos.ownership_kind);
	//set liveliness
	printf("pConf->qos.liveliness.kind[%d]\n", pConf->qos.liveliness.kind);
	printf("pConf->qos.liveliness.lease_duration[%lld]\n", pConf->qos.liveliness.lease_duration);
	//set time_based_filter
	printf("pConf->qos.minimum_separation[%lld]\n", pConf->qos.minimum_separation);
	//set Reliability
	printf("pConf->qos.reliability.kind[%d]\n", pConf->qos.reliability.kind);
	printf("pConf->qos.reliability.max_blocking_time[%lld]\n", pConf->qos.reliability.max_blocking_time);
	//set destination_order
	printf("pConf->qos.destination_order.kind[%d]\n", pConf->qos.destination_order.kind);
	//set History
	printf("pConf->qos.history.kind[%d]\n", pConf->qos.history.kind);
	printf("pConf->qos.history.depth[%d]\n", pConf->qos.history.depth);
	//set default value
	printf("pConf->qos.resource_limits.max_samples[%d]\n", pConf->qos.resource_limits.max_samples);
	printf("pConf->qos.resource_limits.max_instances[%d]\n", pConf->qos.resource_limits.max_instances);
	printf("pConf->qos.resource_limits.max_samples_per_instance[%d]\n", pConf->qos.resource_limits.max_samples_per_instance);

	//log
	//set logLevel
	printf("pConf->log.logLevel[%d]\n", pConf->log.logLevel);
	//set logPath
	printf("pConf->log.logPath[%s]\n", pConf->log.logPath);
	//maxFileSize
	printf("pConf->log.maxFileSize[%d]\n", pConf->log.maxFileSize);
	printf("-------------------printconf-------------------------------------\n");
}

/**
 * @brief 缓冲区的内容转化为配置信息结构体
 *
 * @param buf 存储配置数据的缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * 把缓冲区的内容转化为配置结构体
 */
int  RealtimeNetReadConf_bufToConf(char *buf, RealtimeNetConf * pConf)
{
	printf("call RealtimeNetReadConf_bufToConf ok.\n");
	int ret = 0;

	//license数据转化
	ret = RealtimeNetReadConf_licenseToConf(buf, pConf);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf err\n");
	}

	//通讯协议数据转化
	ret = RealtimeNetReadConf_commToConf(buf, pConf);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf err\n");
	}

	//qos数据转化
	ret = RealtimeNetReadConf_QosToConf(buf, pConf);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf err\n");
	}

	//日志数据转化
	ret = RealtimeNetReadConf_loggingToConf(buf, pConf);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf err\n");
	}

	return 1;
}

/**
 * @brief license信息转化为结构体
 *
 * @param buf 缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * license信息转化为结构体
 */
int  RealtimeNetReadConf_licenseToConf(char *buf, RealtimeNetConf * pConf)
{
	printf("call RealtimeNetReadConf_licenseToConf ok.\n");

	char *key = "license";
	char *info = NULL;

	int ret = 0;

	//get license info
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, key, &info);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get licens err.\n");
		return 0;
	}

	//printf("info is : %s\n", info);

	//get licencePath 
	char *typeKey = "licencePath";
	char *typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(info, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get licencePath err.\n");		
	} else {
		//printf("typeValue is :RealtimeNetReadConf_getInfoFromBuf [%s]\n", typeValue);
		strncpy(pConf->lice.licencePath, typeValue, sizeof(pConf->lice.licencePath) - 1);
		free(typeValue);
		//printf("licencePath is : %s\n", pConf->lice.licencePath);
	}	
	free(info);
	return ret;
}

/**
 * @brief 通讯信息转化为结构体
 *
 * @param buf 缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * 通讯信息转化为结构体
 */
int  RealtimeNetReadConf_commToConf(char *buf, RealtimeNetConf * pConf)
{
//	printf("call RealtimeNetReadConf_commToConf ok.\n");

	char *key = "commProtocol";
	char *info = NULL;

	int ret = 0;

	//get commProtocol info
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, key, &info);
	if (0 == ret) {
		printf("RealtimeNetReadConf_commToConf get commProtocol err.\n");
		return 0;
	}

	//printf("info is : %s\n", info);

	//get Type 
	char *typeKey = "Type";
	char *typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(info, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_commToConf get Type err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converCommProtocol(typeValue, &(pConf->comPro.type));
		free(typeValue);
		//printf("pConf->comPro.type[%d]\n", pConf->comPro.type);
	}

	free(info);
	return ret;
}

/**
 * @brief qos信息转化为结构体
 *
 * @param buf 缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * qos转化为结构体
 */
int  RealtimeNetReadConf_QosToConf(char *buf, RealtimeNetConf * pConf)
{
	printf("call RealtimeNetReadConf_QosToConf ok.\n");

	char *key = "Qos";
	char *info = NULL;

	int ret = 0;

	//get Qos info
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, key, &info);
	if (0 == ret) {
		printf("RealtimeNetReadConf_QosToConf get licens not found.\n");
		return 0;
	}

	//printf("info is : %s", info);

	//todo 20250715 需要添加有效性判断
	//get Durability 
	char *attrName = "valid";
	char *attrValue = NULL;

	char *typeKey = "Durability";
	char *typeValue = NULL;
	char attrStr[128];
	memset(attrStr, 0, sizeof(attrStr));
	pConf->qos.durability_kind_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.durability_kind_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converDurability(typeValue, &(pConf->qos.durability_kind));
		//printf("pConf->qos.durability_kind[%d]\n", pConf->qos.durability_kind);
	}
	else
	{
		printf("RealtimeNetReadConf_QosToConf get Durability not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get Presentation 
	typeKey = "Presentation";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.presentation_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.presentation_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converPresentation(typeValue, &(pConf->qos.presentation));
		free(typeValue);
		//printf("pConf->qos.presentation.access_scope[%d]\n", pConf->qos.presentation.access_scope);
		//printf("pConf->qos.presentation.coherent_access[%d]\n", pConf->qos.presentation.coherent_access);
		//printf("pConf->qos.presentation.ordered_access[%d]\n", pConf->qos.presentation.ordered_access);	
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get Presentation not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get deadline 
	typeKey = "deadline";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.deadline_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.deadline_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converLongLong(typeValue, &(pConf->qos.deadline));
		free(typeValue);
		//printf("pConf->qos.deadline[%lld]\n", pConf->qos.deadline);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get deadline not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get ownership 
	typeKey = "ownership";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.ownership_kind_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.ownership_kind_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converOwnership(typeValue, &(pConf->qos.ownership_kind));
		free(typeValue);
		//printf("pConf->qos.ownership_kind[%d]\n", pConf->qos.ownership_kind);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get ownership not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get liveliness 
	typeKey = "liveliness";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.liveliness_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.liveliness_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converLiveliness(typeValue, &(pConf->qos.liveliness));
		free(typeValue);
		//printf("pConf->qos.liveliness.kind[%d]\n", pConf->qos.liveliness.kind);
		//printf("pConf->qos.liveliness.lease_duration[%lld]\n", pConf->qos.liveliness.lease_duration);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get liveliness not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get time_based_filter 
	typeKey = "time_based_filter";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.time_based_filter_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.time_based_filter_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converLongLong(typeValue, &(pConf->qos.minimum_separation));
		free(typeValue);
		//printf("pConf->qos.minimum_separation[%lld]\n", pConf->qos.minimum_separation);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get time_based_filter not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get partition 
	typeKey = "partition";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.partition_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.partition_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		pConf->qos.partition = typeValue;
		//printf("pConf->qos.partition[%s]\n", pConf->qos.partition);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get partition not found.\n", info);
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get Reliability 
	typeKey = "Reliability";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.reliability_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.reliability_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converReliability(typeValue, &(pConf->qos.reliability));
		free(typeValue);
		//printf("pConf->qos.reliability.kind[%d]\n", pConf->qos.reliability.kind);
		//printf("pConf->qos.reliability.max_blocking_time[%lld]\n", pConf->qos.reliability.max_blocking_time);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get Reliability not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get destination_order 
	typeKey = "destination_order";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.destination_order_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.destination_order_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converDestination_order(typeValue, &(pConf->qos.destination_order.kind));
		free(typeValue);
		//printf("pConf->qos.destination_order.kind[%d]\n", pConf->qos.destination_order.kind);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get destination_order not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get History 
	typeKey = "History";
	typeValue = NULL;
	attrValue = NULL;
	pConf->qos.history_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.history_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converHistory(typeValue, &(pConf->qos.history));
		free(typeValue);
		//printf("pConf->qos.history.kind[%d]\n", pConf->qos.history.kind);
		//printf("pConf->qos.history.depth[%d]\n", pConf->qos.history.depth);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get History not found.\n");
	}
	free(attrValue);
	free(typeValue);
	memset(attrStr, 0, sizeof(attrStr));

	//get resource_limits 
	typeKey = "resource_limits";
	typeValue = NULL;  
	attrValue = NULL;
	pConf->qos.resource_limits_valid = 0;
	if (RealtimeNetReadConf_getAttrFromBuf2(info, typeKey, attrName, &attrValue))
	{
		pConf->qos.resource_limits_valid = str_to_short(attrValue);
		sprintf(attrStr, "%s=\"%s\"", attrName, attrValue);
	}
	if (RealtimeNetReadConf_getInfoFromBuf3(info, typeKey, attrStr, &typeValue))
	{
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converResource_limits(typeValue, &(pConf->qos.resource_limits));
		free(typeValue);
		//printf("pConf->qos.resource_limits.max_samples[%d]\n", pConf->qos.resource_limits.max_samples);
		//printf("pConf->qos.resource_limits.max_instances[%d]\n", pConf->qos.resource_limits.max_instances);
		//printf("pConf->qos.resource_limits.max_samples_per_instance[%d]\n", pConf->qos.resource_limits.max_samples_per_instance);
	}
	else 
	{
		printf("RealtimeNetReadConf_QosToConf get resource_limits not found.\n");
	}	 	  
	free(attrValue);
	free(typeValue);
	//free(attrStr);
	
	free(info);
	return 1;

}

/**
 * @brief log信息转化为结构体
 *
 * @param buf 缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * log转化为结构体
 */
int  RealtimeNetReadConf_loggingToConf(char *buf, RealtimeNetConf * pConf)
{
	printf("call RealtimeNetReadConf_loggingToConf ok.\n");

	char *key = "logging";
	char *info = NULL;

	int ret = 0;

	//get logging info
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, key, &info);
	if (0 == ret) {
		printf("RealtimeNetReadConf_loggingToConf get logging err.\n");
		return 0;
	}

	//printf("info is : %s\n", info);

	//get logLevel 
	char * typeKey = "logLevel";
	char * typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(info, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_loggingToConf get logLevel err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converLogLevel(typeValue, &(pConf->log.logLevel));
		free(typeValue);
		//printf("pConf->log.logLevel[%d]\n", pConf->log.logLevel);
	}

	//get logPath 
	typeKey = "logPath";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(info, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_loggingToConf get logPath err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		strncpy(pConf->log.logPath, typeValue, sizeof(pConf->log.logPath) - 1);
		free(typeValue);
		//printf("pConf->log.logPath[%s]\n", pConf->log.logPath);
	}

	//get maxFileSize 
	typeKey = "maxFileSize";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(info, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_loggingToConf get maxFileSize err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		int fileSize = 0;
		RealtimeNetReadConf_converInt(typeValue, &fileSize);
		//把M转换成字节 1M = 1024k = 1024 *1024字节
		pConf->log.maxFileSize = fileSize * 1024 * 1024; 
		free(typeValue);
		//printf("pConf->log.maxFileSize[%d]\n", pConf->log.maxFileSize);
	}
	return 1;
}

/**
 * @brief 提取xml中的信息
 *
 * @param buf 缓冲区
 * @param key xml中尖括号中的字段
 * @param info 加载字段终中间的值
 * @return 函数调用结果 1-成功 0-失败
 *
 * 提取xml中的信息  
 * 示例：
 *       buf：“<字段>值</字段> ”； 
 *       key：“字段”；
 *       info：“值”
 */
int  RealtimeNetReadConf_getInfoFromBuf(char *buf, char *key, char **info)
{
//	printf("call RealtimeNetReadConf_getInfoFromBuf ok.\n");
	char keyBegin[128];
	char keyEnd[128];
	int keyLength = 0;
	char * beginPos = NULL;
	char * endPos = NULL;
	int infoLength = 0;

	if (NULL == buf) {
//		printf("RealtimeNetReadConf_getInfoFromBuf buf is null\n");
		return 0;
	}


	keyLength = (int)strlen(key);
	if (keyLength > 120) {
		printf("key[%s] is too long\n", key);
		return 0;
	}
	 
	memset(keyBegin, 0, sizeof(keyBegin));
	memset(keyEnd, 0, sizeof(keyEnd));
	sprintf(keyBegin,  "<%s>", key);
	sprintf(keyEnd,  "</%s>", key);


	beginPos = strstr(buf, keyBegin);
	if (0 == beginPos) {
		printf("RealtimeNetReadConf_getInfoFromBuf str[%s] no found.\n", keyBegin);
		return 0;
	}
	endPos = strstr(buf, keyEnd);
	if (0 == endPos) {
		printf("RealtimeNetReadConf_getInfoFromBuf str[%s] no found.\n", keyEnd);
		return 0;
	}

	beginPos = beginPos + keyLength + 2;
	RealtimeNetReadConf_filterSymbols_begin(&beginPos);
	endPos--;
	RealtimeNetReadConf_filterSymbols_end(&endPos);
	endPos++;
	infoLength = (int)(endPos - beginPos);
	if (infoLength <= 0) {
		printf("RealtimeNetReadConf_getInfoFromBuf find info by key[%s] is null.\n", key);
		return 0;
	}

	*info = malloc(infoLength + 1);
	memset(*info, 0, infoLength + 1);
	strncpy(*info, beginPos, infoLength);
	//printf("RealtimeNetReadConf_getInfoFromBuf get info[%s]\n", *info);
	return 1;
}

int RealtimeNetReadConf_getAttrFromBuf2(char *buf, const char *tag, const char *attr, char **value)
{
	char tagOpen[128], *pTag, *pAttr, *pQuote;
	int len;

	if (!buf || !tag || !attr) return 0;

	snprintf(tagOpen, sizeof(tagOpen), "<%s ", tag);
	pTag = strstr(buf, tagOpen);
	if (!pTag)
	{
		snprintf(tagOpen, sizeof(tagOpen), "<%s>", tag);
		pTag = strstr(buf, tagOpen);
		if (pTag) return 0;   /* 无属性 */
	}

	pAttr = strstr(pTag, attr);
	if (!pAttr) return 0;

	pQuote = strchr(pAttr, '=');
	if (!pQuote) return 0;
	++pQuote;
	while (*pQuote == ' ' || *pQuote == '"') ++pQuote;

	pAttr = strchr(pQuote, '"');
	if (!pAttr) return 0;

	len = (int)(pAttr - pQuote);
	*value = (char *)malloc(len + 1);
	if (!*value) return 0;

	strncpy(*value, pQuote, len);
	(*value)[len] = '\0';
	return 1;
}

int RealtimeNetReadConf_getInfoFromBuf3(char *buf, char *tag, char* attr, char **info)
{
	//	printf("call RealtimeNetReadConf_getInfoFromBuf ok.\n");
	char keyBegin[128];
	char keyEnd[128];
	int keyLength = 0;
	char * beginPos = NULL;
	char * endPos = NULL;
	int infoLength = 0;

	if (NULL == buf) {
		//		printf("RealtimeNetReadConf_getInfoFromBuf buf is null\n");
		return 0;
	}


	keyLength = (int)strlen(tag);
	if (keyLength > 120) {
		printf("key[%s] is too long\n", tag);
		return 0;
	}

	memset(keyBegin, 0, sizeof(keyBegin));
	memset(keyEnd, 0, sizeof(keyEnd));
	sprintf(keyBegin, "<%s %s>", tag, attr);
	sprintf(keyEnd, "</%s>", tag);


	beginPos = strstr(buf, keyBegin);
	if (0 == beginPos) {
		printf("RealtimeNetReadConf_getInfoFromBuf str[%s] no found.\n", keyBegin);
		return 0;
	}
	endPos = strstr(buf, keyEnd);
	if (0 == endPos) {
		printf("RealtimeNetReadConf_getInfoFromBuf str[%s] no found.\n", keyEnd);
		return 0;
	}

	beginPos = beginPos + strlen(tag) + 1 + strlen(attr) + 2;
	RealtimeNetReadConf_filterSymbols_begin(&beginPos);
	endPos--;
	RealtimeNetReadConf_filterSymbols_end(&endPos);
	endPos++;
	infoLength = (int)(endPos - beginPos);
	if (infoLength <= 0) {
		printf("RealtimeNetReadConf_getInfoFromBuf find info by key[%s] is null.\n", tag);
		return 0;
	}

	*info = malloc(infoLength + 1);
	memset(*info, 0, infoLength + 1);
	strncpy(*info, beginPos, infoLength);
	//printf("RealtimeNetReadConf_getInfoFromBuf get info[%s]\n", *info);
	return 1;
}

/**
 * @brief 过滤前面的无用符号
 *
 * @param buf 缓冲区
 * @param
 * @return
 *
 * 在缓冲区开始处，过滤掉无用的符号，
 * 这些符号包括空格，回车，换成，tab键
 */
void	 RealtimeNetReadConf_filterSymbols_begin(char ** buf)
{
	char * ptr = *buf;

	if (NULL == ptr)
		return;

	//回车，换行，空格和tab去掉
	while ((0x0a == *ptr) || (0x0d == *ptr) || (0x20 == *ptr) || (0x09 == *ptr))
		ptr++;

	*buf = ptr;

}

/**
 * @brief 过滤后面的无用符号
 *
 * @param buf 缓冲区
 * @param
 * @return
 *
 * 在缓冲区结束处，反向过滤掉无用的符号，
 * 这些符号包括空格，回车，换成，tab键
 */
void	 RealtimeNetReadConf_filterSymbols_end(char ** buf)
{
	char * ptr = *buf;

	if (NULL == ptr)
		return;

	while ((0x0a == *ptr) || (0x0d == *ptr) || (0x20 == *ptr) || (0x09 == *ptr))
		ptr--;

	*buf = ptr;

}

/**
 * @brief 转换通信协议内容
 *
 * @param buf 缓冲区
 * @param type 通信协议内容
 * @return
 *
 * 根据缓冲区的字符串，转化成通信协议内容
 */
void RealtimeNetReadConf_converCommProtocol(char * buf, int * type)
{
	if (0 == strcmp(buf, "udp")) {
		*type = 0;
	} else if (0 == strcmp(buf, "tcp")) {
		*type = 1;
	} else if (0 == strcmp(buf, "shm")) {
		*type = 2;
	} else {
		*type = 0;
	}
}

/**
 * @brief 转换durability内容
 *
 * @param buf 缓冲区
 * @param type durability内容
 * @return
 *
 * 根据缓冲区的字符串，转化成durability内容
 */
void RealtimeNetReadConf_converDurability(char * buf, dds_durability_kind_t * type)
{
	if (0 == strcmp(buf, "VOLATILE")) {
		*type = DDS_DURABILITY_VOLATILE;
	}
	else if (0 == strcmp(buf, "TRANSIENT_LOCAL")) {
		*type = DDS_DURABILITY_TRANSIENT_LOCAL;
	}
	else if (0 == strcmp(buf, "TRANSIENT")) {
		*type = DDS_DURABILITY_TRANSIENT;
	}
	else if (0 == strcmp(buf, "PERSISTENT")) {
		*type = DDS_DURABILITY_PERSISTENT;
	}
	else {
		*type = DDS_DURABILITY_VOLATILE;
	}

}

/**
 * @brief 转换presentation内容
 *
 * @param buf 缓冲区
 * @param type presentation内容
 * @return
 *
 * 根据缓冲区的字符串，转化成presentation内容
 */
void RealtimeNetReadConf_converPresentation(char * buf, dds_presentation_qospolicy_t * type)
{
	int ret = 0;
	//get access_scope 
	char *typeKey = "access_scope";
	char *typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get Durability err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converAccess_scope(typeValue, &(type->access_scope));
		free(typeValue);
		//printf("type->access_scope[%d]\n", type->access_scope);
	}

	//get access_scope 
	typeKey = "coherent_access";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get Durability err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converBool(typeValue, &(type->coherent_access));
		free(typeValue);
		//printf("type->coherent_access[%d]\n", type->coherent_access);
	}

	//get access_scope 
	typeKey = "ordered_access";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get Durability err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converBool(typeValue, &(type->ordered_access));
		free(typeValue);
		//printf("type->ordered_access[%d]\n", type->ordered_access);
	}
}

/**
 * @brief 转换access_scope内容
 *
 * @param buf 缓冲区
 * @param type access_scope内容
 * @return
 *
 * 根据缓冲区的字符串，转化成access_scope内容
 */
void RealtimeNetReadConf_converAccess_scope(char * buf, dds_presentation_access_scope_kind_t * type)
{
	if (0 == strcmp(buf, "INSTANCE")) {
		*type = DDS_PRESENTATION_INSTANCE;
	}
	else if (0 == strcmp(buf, "TOPIC")) {
		*type = DDS_PRESENTATION_TOPIC;
	}
	else if (0 == strcmp(buf, "GROUP")) {
		*type = DDS_PRESENTATION_GROUP;
	}
	else {
		*type = DDS_PRESENTATION_INSTANCE;
	}

}

/**
 * @brief 转换bool内容
 *
 * @param buf 缓冲区
 * @param type bool内容
 * @return
 *
 * 根据缓冲区的字符串，转化成bool内容
 */
void RealtimeNetReadConf_converBool(char * buf, unsigned char * type)
{
	if (0 == strcmp(buf, "true")) {
		*type = true;
	}
	else {
		*type = false;
	}


}

/**
 * @brief 转换int内容
 *
 * @param buf 缓冲区
 * @param type int内容
 * @return
 *
 * 根据缓冲区的字符串，转化成int内容
 */
void RealtimeNetReadConf_converInt(char * buf, int * type)
{
	*type = atoi(buf);
}

/**
 * @brief 转换longlong内容
 *
 * @param buf 缓冲区
 * @param type longlong内容
 * @return
 *
 * 根据缓冲区的字符串，转化成longlong内容
 */
void RealtimeNetReadConf_converLongLong(char * buf, int64_t * type)
{
	*type = str_to_int64(buf);

}

/**
 * @brief 转换ownership内容
 *
 * @param buf 缓冲区
 * @param type ownership内容
 * @return
 *
 * 根据缓冲区的字符串，转化成ownership内容
 */
void RealtimeNetReadConf_converOwnership(char * buf, dds_ownership_kind_t * type)
{
	if (0 == strcmp(buf, "EXCLUSIVE")) {
		*type = DDS_OWNERSHIP_EXCLUSIVE;
	}
	else {
		*type = DDS_OWNERSHIP_SHARED;
	}

}

/**
 * @brief 转换liveliness内容
 *
 * @param buf 缓冲区
 * @param type liveliness内容
 * @return
 *
 * 根据缓冲区的字符串，转化成liveliness内容
 */
void RealtimeNetReadConf_converLiveliness(char * buf, dds_liveliness_qospolicy_t * type)
{
	int ret = 0;
	//get kind 
	char *typeKey = "kind";
	char *typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get kind err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converLiveliness_kind(typeValue, &(type->kind));
		free(typeValue);
		//printf("type->kind[%d]\n", type->kind);
	}

	//get lease_duration 
	typeKey = "lease_duration";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get lease_duration err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converLongLong(typeValue, &(type->lease_duration));
		free(typeValue);
		if (0 == type->lease_duration) {
			type->lease_duration = DDS_INFINITY;
		}
		//printf("type->lease_duration[%lld]\n", type->lease_duration);
	}

}

/**
 * @brief 转换liveliness_kind内容
 *
 * @param buf 缓冲区
 * @param type liveliness_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成liveliness_kind内容
 */
void RealtimeNetReadConf_converLiveliness_kind(char * buf, dds_liveliness_kind_t * type)
{
	if (0 == strcmp(buf, "AUTOMATIC")) {
		*type = DDS_LIVELINESS_AUTOMATIC;
	}
	else if (0 == strcmp(buf, "MANUAL_BY_PARTICIPANT")) {
		*type = DDS_LIVELINESS_MANUAL_BY_PARTICIPANT;
	}
	else if (0 == strcmp(buf, "MANUAL_BY_TOPIC")) {
		*type = DDS_LIVELINESS_MANUAL_BY_TOPIC;
	}
	else {
		*type = DDS_LIVELINESS_AUTOMATIC;
	}

}

/**
 * @brief 转换reliability_qospolicy内容
 *
 * @param buf 缓冲区
 * @param type reliability_qospolicy内容
 * @return
 *
 * 根据缓冲区的字符串，转化成reliability_qospolicy内容
 */
void RealtimeNetReadConf_converReliability(char * buf, dds_reliability_qospolicy_t * type)
{
	int ret = 0;
	//get kind 
	char *typeKey = "kind";
	char *typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get reliability err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converReliability_kind(typeValue, &(type->kind));
		free(typeValue);
		//printf("type->kind[%d]\n", type->kind);
	}

	//get max_blocking_time 
	typeKey = "max_blocking_time";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get max_blocking_time err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converLongLong(typeValue, &(type->max_blocking_time));
		free(typeValue);
		//printf("type->max_blocking_time[%lld]\n", type->max_blocking_time);
	}
}

/**
 * @brief 转换reliability_kind内容
 *
 * @param buf 缓冲区
 * @param type reliability_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成reliability_kind内容
 */
void RealtimeNetReadConf_converReliability_kind(char * buf, dds_reliability_kind_t * type)
{
	if (0 == strcmp(buf, "BEST_EFFORT")) {
		*type = DDS_RELIABILITY_BEST_EFFORT;
	}
	else if (0 == strcmp(buf, "RELIABLE")) {
		*type = DDS_RELIABILITY_RELIABLE;
	}
	else {
		*type = DDS_RELIABILITY_BEST_EFFORT;
	}
}

/**
 * @brief 转换destination_order_kind内容
 *
 * @param buf 缓冲区
 * @param type destination_order_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成destination_order_kind内容
 */
void RealtimeNetReadConf_converDestination_order(char * buf, dds_destination_order_kind_t * type)
{
	if (0 == strcmp(buf, "BY_RECEPTION_TIMESTAMP")) {
		*type = DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP;
	}
	else if (0 == strcmp(buf, "BY_SOURCE_TIMESTAMP")) {
		*type = DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP;
	}
	else {
		*type = DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP;
	}
}

/**
 * @brief 转换history_kind内容
 *
 * @param buf 缓冲区
 * @param type history_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成history_kind内容
 */
void RealtimeNetReadConf_converHistory(char * buf, dds_history_qospolicy_t * type)
{
	int ret = 0;
	//get kind 
	char *typeKey = "kind";
	char *typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get history err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converHistory_kind(typeValue, &(type->kind));
		free(typeValue);
		//printf("type->kind[%d]\n", type->kind);
	}

	//get lease_duration 
	typeKey = "depth";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_converHistory get depth err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converInt(typeValue, &(type->depth));
		free(typeValue);
		//printf("type->depth[%d]\n", type->depth);
	}

}

/**
 * @brief 转换reliability_kind内容
 *
 * @param buf 缓冲区
 * @param type reliability_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成reliability_kind内容
 */
void RealtimeNetReadConf_converHistory_kind(char * buf, dds_history_kind_t * type)
{
	if (0 == strcmp(buf, "KEEP_LAST")) {
		*type = DDS_HISTORY_KEEP_LAST;
	}
	else if (0 == strcmp(buf, "KEEP_ALL")) {
		*type = DDS_HISTORY_KEEP_ALL;
	}
	else {
		*type = DDS_HISTORY_KEEP_LAST;
	}

}

/**
 * @brief 转换converResource_limits内容
 *
 * @param buf 缓冲区
 * @param type converResource_limits内容
 * @return
 *
 * 根据缓冲区的字符串，转化成converResource_limits内容
 */
void RealtimeNetReadConf_converResource_limits(char * buf, dds_resource_limits_qospolicy_t * type)
{
	int ret = 0;
	//get access_scope 
	char *typeKey = "max_samples";
	char *typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_licenseToConf get Durability err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converInt(typeValue, &(type->max_samples));
		free(typeValue);
		//printf("type->max_samples[%d]\n", type->max_samples);
	}

	//get access_scope 
	typeKey = "max_instances";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_converResource_limits get max_instances err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converInt(typeValue, &(type->max_instances));
		free(typeValue);
		//printf("type->max_instances[%d]\n", type->max_instances);
	}

	//get access_scope 
	typeKey = "max_samples_per_instance";
	typeValue = NULL;
	ret = RealtimeNetReadConf_getInfoFromBuf(buf, typeKey, &typeValue);
	if (0 == ret) {
		printf("RealtimeNetReadConf_converResource_limits get max_samples_per_instance err.\n");
	}
	else {
		//printf("typeValue is : [%s]\n", typeValue);
		RealtimeNetReadConf_converInt(typeValue, &(type->max_samples_per_instance));
		free(typeValue);
		//printf("type->max_samples_per_instance[%d]\n", type->max_samples_per_instance);
	}


}

/**
 * @brief 转换converLogLevel内容
 *
 * @param buf 缓冲区
 * @param type converLogLevel内容
 * @return
 *
 * 根据缓冲区的字符串，转化成converLogLevel内容
 */
void RealtimeNetReadConf_converLogLevel(char * buf, int * type)
{

	if (0 == strcmp(buf, "NO")) {
		*type = 0;
	}
	else if (0 == strcmp(buf, "DEBUG")) {
		*type = 1;
	}
	else if (0 == strcmp(buf, "INFO")) {
		*type = 2;
	}
	else if (0 == strcmp(buf, "WARNING")) {
		*type = 3;
	}
	else if (0 == strcmp(buf, "ERROR")) {
		*type = 4;
	}
	else if (0 == strcmp(buf, "FATAL")) {
		*type = 5;
	}
	else {
		*type = 0;
	}


}

short str_to_short(const char * val)
{
	if (!val) return 0;          // 空指针
	char *endptr = NULL;
	long tmp = strtol(val, &endptr, 10);

	/* 检查范围与转换完整性 */
	if (*endptr != '\0' || tmp < SHRT_MIN || tmp > SHRT_MAX)
		return 0;

	return tmp;   // 成功
}
