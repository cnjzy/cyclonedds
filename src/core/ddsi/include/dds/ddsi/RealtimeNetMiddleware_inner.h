#ifndef REALTIME_NET_MIDDLEWARE_INNER_H
#define REALTIME_NET_MIDDLEWARE_INNER_H

#include "dds/ddsrt/threads.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/time.h"
#include "dds/ddsi/ddsi_config.h"

#include "os_defs.h"

#include "RealtimeNetMiddleware.h"
#include "RealtimeNetConf.h"

#if defined (__cplusplus)
extern "C" {
#endif	

#pragma pack(1) /*1字节对齐*/

typedef struct ST_LogSetting_tag
{
	/* 日志信息 */
	log_handle* pad_log_handle_fp;
	os_log_attr_t log_attr;
} LogSetting;


//每一个topic对应的结构体
typedef struct ST_TopicMapInnerItem_tag
{
	char *		topicName;				// 通过DDS发布/订阅信息使用到的TopicName
	int			offset;					// 原反射内存网的Offset
	int			size;					// TopicName对应数据的缓冲区大小
	int			status;					// 该TopicName是读还是写，该字段主要为了兼容反射内存网
	dds_entity_t topic;
	dds_entity_t operator;				//writer or reader
	char *		buf;					//存储最后一条数据的缓冲区
	int         num;					//当前item的数组序号
	bool		haveDataFlg;			//是否有数据
}TopicMapInnerItem;

//每个示例participanet对应的整个结构体
typedef struct ST_RtNetHandle_tag
{
	int					ddsDominId;			// dds域id
	char *				networkSegment;		// dds初始化使用的网段ip（当机器有多个网卡时，需要确保该网段的设备能收到dds信息），机器只有单个网卡时可为空
	TopicMapInnerItem *	topicMapList;		// dds注册的topic信息列表，因为要兼容反射内存网的offset，所以初始化时传入TopicName和Offset的映射表
	int					topicMapListLength;	// 映射表的长度
	OnReceiverDataFun	onReceiverTopic;	// 接收信息的回调，通过函数指针实现。外部传入初始化结构体时，可以传入该变量，这样外部可以通过回调异步接收数据。如果外部传空，
	int					timeout;			// 接收消息的超时时间，单位秒

	dds_entity_t		participant;		// 参与者
	dds_entity_t		waitset;			// 消息触发条件

	dds_entity_t		topicReq;			// 发送心跳请求的topic
	dds_entity_t		topicRes;			// 接收心跳应答的topic
	dds_entity_t		topicNode;			// 接收节点信息的topic
	dds_entity_t		readerReq;			// 发布心跳请求的实体
	dds_entity_t		writerRes;			// 接收心跳应答的实体
	dds_entity_t		writerNode;			// 接收节点信息的实体
	int					seq;				// 监控心跳的ID,如发生变化重新发送node信息

	ddsrt_thread_t		tid;				// 内部接收消息的线程ID
	bool				exitFlg;			// 退出标志
	bool				recvThreadExitFlg;	// 接收线程退出标志
	struct ddsrt_hh *	offsetReadHash;		// 存储item的哈希handle by  Read offset
	struct ddsrt_hh *	offsetWriteHash;	// 存储item的哈希handle by  Write offset
	struct ddsrt_hh *	readerHash;			// 存储item的哈希handle by reader

	ddsrt_mutex_t		lock;				// 内部数据拷贝锁
	struct ddsi_config	raw;				// dds的配置信息
	int					processId;			// 进程id
	int					threadId;			// 线程id
	char 				ipAddr[32];			// ip地址
	char				ip;					// ip地址的最后一个值
	LogSetting *		log;				// 日志句柄
	RealtimeNetConf *	pConf;				// qos设置
	dds_qos_t *			qos;				// dds内部qos

}RtNetHandle;


#pragma pack()/*还原默认对齐*/

/**
 * @brief 初始化网络参数并设置到句柄中
 *
 * @param initParam 初始化参数，包含网络段、Topic映射列表等信息
 * @param handle 句柄，用于存储初始化后的参数
 *
 * 该函数将`initParam`中的参数复制到`handle`中，并分配必要的内存。
 * 如果未设置超时时间，则默认设置为1小时。
 */
void RealtimeNetSetParam(RtNetInitParam *initParam, RtNetHandle * handle);


/**
 * @brief 初始化哈希表，用于存储TopicMapInnerItem
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1
 *
 * 该函数创建一个哈希表，并将Topic映射列表中的每个元素添加到哈希表中。
 */
int RealtimeNetInitHash(RtNetHandle *handle);

/**
 * @brief 创建DDS参与者
 *
 * @param handle 句柄，包含网络段信息
 * @param myQos  Qos
 * @return int 成功返回1，失败返回0
 *
 * 该函数初始化DDS配置，创建参与者，并将其存储到句柄中。
 */
int RealtimeNetCreatePart(RtNetHandle *handle, Qos * myQos);

/**
 * @brief 创建Topic和操作器（Reader/Writer）
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1，失败返回0
 *
 * 该函数为每个Topic创建对应的Reader或Writer，并将其存储到句柄中。
 */
int RealtimeNetCreateTopicAndOper(RtNetHandle *handle);

/**
 * @brief 创建Topic和操作器（Reader/Writer）
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1，失败返回0
 *
 * 该函数为每个Topic创建对应的Reader或Writer，并将其存储到句柄中。
 */
int RealtimeNetCreateNormalTopicAndOper(RtNetHandle *handle);

/**
 * @brief 创建Topic和操作器（Reader/Writer）
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1，失败返回0
 *
 * 该函数为每个Topic创建对应的Reader或Writer，并将其存储到句柄中。
 */
int RealtimeNetCreatMonitorTopicAndOper(RtNetHandle *handle);

/**
 * @brief 创建条件变量并注册到WaitSet
 *
 * @param handle 句柄，包含Reader列表
 * @return int 成功返回1
 *
 * 该函数为每个Reader创建ReadCondition，并将其注册到WaitSet中。
 */
int RealtimeNetCreateCond(RtNetHandle *handle);

/**
 * @brief 消息接收线程，用于处理接收到的DDS消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetMsgThread(void * inPara);


/**
 * @brief 创建消息接收线程
 *
 * @param handle 句柄，包含Reader列表
 * @return int 成功返回1
 *
 * 如果存在Reader，则创建一个线程用于接收消息。
 */
int RealtimeNetCreateThread(RtNetHandle *handle);

/**
 * @brief 查找Topic by read offset
 *
 * @param handle 句柄
 * @param offset 偏移量
 * @return int 成功返回Topic索引，失败返回-1
 *
 * 根据偏移量在哈希表中查找对应的Topic。
 */
int RealtimeNetFindTopicbyReadOffset(RtNetHandle *handle, unsigned int offset);

/**
 * @brief 查找Topic by write offset
 *
 * @param handle 句柄
 * @param offset 偏移量
 * @return int 成功返回Topic索引，失败返回-1
 *
 * 根据偏移量在哈希表中查找对应的Topic。
 */
int RealtimeNetFindTopicbyWriteOffset(RtNetHandle *handle, unsigned int offset);

/**
 * @brief 查找Topic by reader
 *
 * @param handle 句柄
 * @param reader 哈希索引
 * @return int 成功返回Topic索引，失败返回-1
 *
 * 根据偏移量在哈希表中查找对应的Topic。
 */
int RealtimeNetFindTopicbyReader(RtNetHandle *handle, unsigned int reader);


/**
 * @brief	鉴权
 *
 * @return	函数调用结果 1-成功 0-失败
 */
int RealtimeNetAuthentication(char * filePath);

/**
 * @brief 把文件中的内容加载到缓冲区
 *
 * @param fname 配置文件全路径
 * @param buf 保存配置文件内容的缓冲区
 * @param bufLen 缓冲区大小
 * @return 函数调用结果 1-成功 0-失败
 *
 * 读取配置文件中的内容，把文件的内容拷贝到缓冲区
 */
int  RealtimeNetReadFile(char * fname, char *buf, int bufLen);

/**
 * @brief 处理接收到的正常DDS消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetDoNormalMsg(RtNetHandle *handle, int reader, void * msg);

/**
 * @brief 处理接收到的监控DDS消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetDoMonitalMsg(RtNetHandle *handle, void * msg);


/**
 * @brief 发送监测的心跳应答消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 
 */
int RealtimeNetSendMonitalResMsg(RtNetHandle *handle);


/**
 * @brief 发送检测的节点消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 
 */
int RealtimeNetSendMonitalNodeMsg(RtNetHandle *handle);

/**
 * @brief 获取ip地址
 *
 * @param addr ip地址
 * @return 
 *
 *
 */
void get_local_ips(char * addr);

/**
 * @brief 获取ip地址最后.后面的数字
 *
 * @param  addr ip地址
 * @return .后面的数字
 *
 *
 */
char getIpLastChar(const char *ip);

/**
 * @brief	日志初始化
 *
 * @param handle 句柄
 * @return	函数调用结果 1-成功 0-失败
 */
int LogInit(RtNetHandle *handle, os_log_level level);

/**
 * @brief	设置日志级别
 *
 *
 * @param handle 句柄
 * @param	level	日志级别
 * @return	函数调用结果 1-成功 0-失败
 */
int setLogLevel(RtNetHandle *handle, os_log_level level);

/**
 * @brief	设置日志文件
 *
 * @param handle 句柄
 * @param	filePath	日志级别
 * @param	fileName	日志级别
 * @param	fileMaxSize	日志文件最大大小（字节）
 * @return	函数调用结果 1-成功 0-失败
 */
int setLogFile(RtNetHandle *handle, char filePath[256], char fileName[256], int fileMaxSize);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_INNER_H
