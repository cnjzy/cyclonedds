#ifndef REALTIME_NET_CONF_H
#define REALTIME_NET_CONF_H

#include "dds/dds.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/ddsc/dds_public_qosdefs.h"


#if defined (__cplusplus)
extern "C" {
#endif
	//license相关配置
	typedef struct ST_licence_tag
	{
		char 						licencePath[128];		// license配置文件路径
	}licence;

	//通讯协议相关配置
	typedef struct ST_commProtocol_tag
	{
		int			type;					// 0：udp； 1：tcp； 2：shm
	}commProtocol;
	
	//qos相关配置
	typedef struct ST_Qos_tag
	{
		short							durability_kind_valid;		// 持久性是否有效
		dds_durability_kind_t			durability_kind;			// 持久性	dds_qset_durability
		short							presentation_valid;			// 顺序是否有效
		dds_presentation_qospolicy_t	presentation;				// 顺序		dds_qset_presentation
		short							deadline_valid;				// 截止时间是否有效
		int64_t							deadline;					// 截止时间 dds_qset_deadline
		short							ownership_kind_valid;		// 所有权是否有效
		dds_ownership_kind_t			ownership_kind;				// 所有权   dds_qset_ownership
		short							liveliness_valid;			// 活力是否有效
		dds_liveliness_qospolicy_t		liveliness;					// 活力		dds_qset_liveliness
		short							time_based_filter_valid;	// 基于时间的过滤器是否有效
		dds_duration_t					minimum_separation;			// 基于时间的过滤器   dds_qset_time_based_filter
		short							partition_valid;			// 分区是否有效
		char *							partition;					// 分区		dds_qset_partition1
		short							reliability_valid;			// 可靠性是否有效
		dds_reliability_qospolicy_t		reliability;				// 可靠性	dds_qset_reliability
		short							destination_order_valid;	// 目的地订单是否有效
		dds_destination_order_qospolicy_t destination_order;		// 目的地订单	dds_qset_destination_order
		short							history_valid;				// 历史数据是否有效
		dds_history_qospolicy_t			history;					// 历史数据	dds_qset_history
		short							resource_limits_valid;		// 资源限制是否有效
		dds_resource_limits_qospolicy_t resource_limits;			// 资源限制	dds_qset_resource_limits

	}Qos;

	//日志相关配置
	typedef struct ST_logging_tag
	{
		int							logLevel;
		char						logPath[128];		// 日志文件路径
		int							maxFileSize;			//日志文件大小
	}logging;

	//整体配置
	typedef struct ST_RealtimeNetConf_tag
	{
		licence						lice;					// license
		commProtocol				comPro;					// 通信协议
		Qos							qos;					// Qos
		logging						log;					// 日志
	}RealtimeNetConf;

/**
 * @brief 读取配置
 *
 * @param path 配置文件的全路径
 * @param pConf 读取到的配置信息
 * @return int 成功返回1，失败返回0
 *
 * 把配置文件中的配置加载到配置信息结构体中。不存在的用默认值
 */
int  RealtimeNetReadConf(char * path, RealtimeNetConf * pConf);


/**
 * @brief 申请配置信息内存
 *
 * @param 
 * @param 
 * @return 指向配置信息的结构体指针
 *
 * 为配置信息结构体申请内存，并初始化为0
 */
RealtimeNetConf *  RealtimeNetReadConf_malloc();

/**
 * @brief 释放配置信息内存
 *
 * @param pConf 配置信息指针
 * @param
 * @return 无
 *
 * 为配置信息结构体释放内存，内部的指针也释放内存
 */
void  RealtimeNetReadConf_free(RealtimeNetConf * pConf);

/**
 * @brief 把配置文件中的内容加载到缓冲区
 *
 * @param fname 配置文件全路径
 * @param buf 保存配置文件内容的缓冲区
 * @return 函数调用结果 1-成功 0-失败
 *
 * 读取配置文件中的内容，申请内存，把文件的内容拷贝到内存
 */
int  RealtimeNetReadConf_fileToBuf(char * fname, char **buf);


/**
 * @brief 初始化配置信息
 *
 * @param pConf 配置信息结构体
 * @param 
 * @return 
 *
 * 给配置信息赋默认值
 */
void RealtimeNetReadConf_initConf(RealtimeNetConf * pConf);

/**
 * @brief 设置QOS信息
 *
 * @param qos dds的qos结构体
 * @param myQos 业务定义的qos结构体
 * @return
 *
 * 把业务定义的qos信息赋值给dds的qos结构体
 */
void RealtimeNetReadConf_setQos(dds_qos_t * qos, Qos * myQos);

/**
 * @brief 缓冲区的内容转化为配置信息结构体
 *
 * @param buf 存储配置数据的缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * 把缓冲区的内容转化为配置结构体
 */
int  RealtimeNetReadConf_bufToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief 打印配置信息结构体
 *
 * @param pConf 配置信息结构体
 * @param 
 * @return 
 *
 * 打印配置信息结构体
 */
void RealtimeNetReadConf_printConf(RealtimeNetConf * pConf);

/**
 * @brief license信息转化为结构体
 *
 * @param buf 缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * license信息转化为结构体
 */
int  RealtimeNetReadConf_licenseToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief 通讯信息转化为结构体
 *
 * @param buf 缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * 通讯信息转化为结构体
 */
int  RealtimeNetReadConf_commToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief qos信息转化为结构体
 *
 * @param buf 缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * qos转化为结构体
 */
int  RealtimeNetReadConf_QosToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief log信息转化为结构体
 *
 * @param buf 缓冲区
 * @param pConf 配置信息结构体
 * @return 函数调用结果 1-成功 0-失败
 *
 * log转化为结构体
 */
int  RealtimeNetReadConf_loggingToConf(char *buf, RealtimeNetConf * pConf);

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
int  RealtimeNetReadConf_getInfoFromBuf(char *buf, char *key, char **info);

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
 *       buf：“<字段 属性=值>内容</字段> ”；
 *       tag：“字段”；
 *       attr：“属性”；
 *       value：“值”
 */
int RealtimeNetReadConf_getAttrFromBuf2(char *buf, const char *tag, const char *attr, char **value);

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
int  RealtimeNetReadConf_getInfoFromBuf3(char *buf, char *key, char* attr, char **info);

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
void RealtimeNetReadConf_filterSymbols_begin(char ** buf);

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
void RealtimeNetReadConf_filterSymbols_end(char ** buf);

/**
 * @brief 转换通信协议内容
 *
 * @param buf 缓冲区
 * @param type 通信协议内容
 * @return 
 *
 * 根据缓冲区的字符串，转化成通信协议内容
 */
void RealtimeNetReadConf_converCommProtocol(char * buf, int * type);

/**
 * @brief 转换durability内容
 *
 * @param buf 缓冲区
 * @param type durability内容
 * @return
 *
 * 根据缓冲区的字符串，转化成durability内容
 */
void RealtimeNetReadConf_converDurability(char * buf, dds_durability_kind_t * type);

/**
 * @brief 转换presentation内容
 *
 * @param buf 缓冲区
 * @param type presentation内容
 * @return
 *
 * 根据缓冲区的字符串，转化成presentation内容
 */
void RealtimeNetReadConf_converPresentation(char * buf, dds_presentation_qospolicy_t * type);

/**
 * @brief 转换access_scope内容
 *
 * @param buf 缓冲区
 * @param type access_scope内容
 * @return
 *
 * 根据缓冲区的字符串，转化成access_scope内容
 */
void RealtimeNetReadConf_converAccess_scope(char * buf, dds_presentation_access_scope_kind_t * type);
	
/**
 * @brief 转换bool内容
 *
 * @param buf 缓冲区
 * @param type bool内容
 * @return
 *
 * 根据缓冲区的字符串，转化成bool内容
 */
void RealtimeNetReadConf_converBool(char * buf, unsigned char * type);

/**
 * @brief 转换int内容
 *
 * @param buf 缓冲区
 * @param type int内容
 * @return
 *
 * 根据缓冲区的字符串，转化成int内容
 */
void RealtimeNetReadConf_converInt(char * buf, int * type);

/**
 * @brief 转换longlong内容
 *
 * @param buf 缓冲区
 * @param type longlong内容
 * @return
 *
 * 根据缓冲区的字符串，转化成longlong内容
 */
void RealtimeNetReadConf_converLongLong(char * buf, int64_t * type);

/**
 * @brief 转换ownership内容
 *
 * @param buf 缓冲区
 * @param type ownership内容
 * @return
 *
 * 根据缓冲区的字符串，转化成ownership内容
 */
void RealtimeNetReadConf_converOwnership(char * buf, dds_ownership_kind_t * type);

/**
 * @brief 转换liveliness_qospolicy内容
 *
 * @param buf 缓冲区
 * @param type liveliness_qospolicy内容
 * @return
 *
 * 根据缓冲区的字符串，转化成liveliness_qospolicy内容
 */
void RealtimeNetReadConf_converLiveliness(char * buf, dds_liveliness_qospolicy_t * type);

/**
 * @brief 转换liveliness_kind内容
 *
 * @param buf 缓冲区
 * @param type liveliness_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成liveliness_kind内容
 */
void RealtimeNetReadConf_converLiveliness_kind(char * buf, dds_liveliness_kind_t * type);

/**
 * @brief 转换reliability_qospolicy内容
 *
 * @param buf 缓冲区
 * @param type reliability_qospolicy内容
 * @return
 *
 * 根据缓冲区的字符串，转化成reliability_qospolicy内容
 */
void RealtimeNetReadConf_converReliability(char * buf, dds_reliability_qospolicy_t * type);

/**
 * @brief 转换reliability_kind内容
 *
 * @param buf 缓冲区
 * @param type reliability_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成reliability_kind内容
 */
void RealtimeNetReadConf_converReliability_kind(char * buf, dds_reliability_kind_t * type);

/**
 * @brief 转换destination_order_kind内容
 *
 * @param buf 缓冲区
 * @param type destination_order_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成destination_order_kind内容
 */
void RealtimeNetReadConf_converDestination_order(char * buf, dds_destination_order_kind_t * type);
	
/**
 * @brief 转换history_qospolicy内容
 *
 * @param buf 缓冲区
 * @param type history_qospolicy内容
 * @return
 *
 * 根据缓冲区的字符串，转化成history_qospolicy内容
 */
void RealtimeNetReadConf_converHistory(char * buf, dds_history_qospolicy_t * type);

/**
 * @brief 转换history_kind内容
 *
 * @param buf 缓冲区
 * @param type history_kind内容
 * @return
 *
 * 根据缓冲区的字符串，转化成history_kind内容
 */
void RealtimeNetReadConf_converHistory_kind(char * buf, dds_history_kind_t * type);

/**
 * @brief 转换converResource_limits内容
 *
 * @param buf 缓冲区
 * @param type converResource_limits内容
 * @return
 *
 * 根据缓冲区的字符串，转化成converResource_limits内容
 */
void RealtimeNetReadConf_converResource_limits(char * buf, dds_resource_limits_qospolicy_t * type);

/**
 * @brief 转换converLogLevel内容
 *
 * @param buf 缓冲区
 * @param type converLogLevel内容
 * @return
 *
 * 根据缓冲区的字符串，转化成converLogLevel内容
 */
void RealtimeNetReadConf_converLogLevel(char * buf, int * type);

/**
 * @brief 将字符串转换为short
 *
 * @param val 字符串
 * @return	short
 */
short str_to_short(const char *val);
	
#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_CONF_H