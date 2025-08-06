#ifndef REALTIME_NET_MIDDLEWARE_MONITOR_INNER_H
#define REALTIME_NET_MIDDLEWARE_MONITOR_INNER_H

#include "dds/dds.h"
#include "os_defs.h"
#include "dds/ddsrt/threads.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/time.h"
#include "dds/ddsi/RealtimeNetMonitor.h"
#include "dds/ddsi/RealtimeNetMontorData.h"

#include "dds/ddsrt/hopscotch.h"
#include "dds/ddsi/ddsi_config.h"

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif
extern const char * 			c_topicNameReq;		// 发送请求的topic名
extern const char * 			c_topicNameRes;		// 接收应答的topic名
extern const char * 			c_topicNameNode;	// 接收节点的topic名


//每一个topic对应的结构体
typedef struct ST_MoitorTopicItem_tag
{
	char *					topicName;				// 通过DDS发布/订阅信息使用到的TopicName
	dds_entity_t			topic;					// topic
	dds_entity_t			reader;					// reader
	void *					nextTopic;				// 指向下一个topic
}MoitorTopicItem;

typedef struct ST_RtNetMonitorHandle_tag
{
	OnReceiverNodeFun		onReceiverNode;			// 节点信息回调函数
	OnReceiverHeartBeatFun	onReceiverRes;			// 心跳信息回调函数
	OnReceiverDataFun		OnReceiverData;			// 业务消息的回调函数
	int						timeout;				// 心跳消息的频率，单位秒

	dds_entity_t			participant;			// 参与者
	dds_entity_t			waitset;				// 消息触发条件
	ddsrt_mutex_t			lock;					// 动态添加topic拷贝锁
	struct ddsrt_hh *		readerHash;				// 存储item的哈希handle by reader
	MoitorTopicItem *		topicHead;				// 指向topic链表的指针

	dds_entity_t			topicReq;				// 发送心跳请求的topic
	dds_entity_t			topicRes;				// 接收心跳应答的topic
	dds_entity_t			topicNode;				// 接收节点信息的topic

	dds_entity_t			writerReq;				// 发布心跳请求的实体
	dds_entity_t			readerRes;				// 接收心跳应答的实体
	dds_entity_t			readerNode;				// 接收节点信息的实体

	bool					exitFlg;				// 退出标志，通知两个线程退出
	bool					sendExitFlg;			// 发送线程退出成功标志
	bool					recvExitFlg;			// 接收线程退出成功标志

	ddsrt_thread_t			sendTid;				// 内部发送消息的线程ID
	ddsrt_thread_t			recvTid;				// 内部接收消息的线程ID
}RtNetMonitorHandle;

/*
 * @brief 初始化handle
 *
 * @param handle 内部全局结构体域id
 * @return int 成功返回1，失败返回0
 *
 * 该函数初始化DDS配置，创建参与者。
 */
int RealtimeNetMonitorInitHandle(RtNetMonitorHandle * handle);

/*
 * @brief 创建DDS参与者
 *
 * @param ddsDominId dds域id
 * @param networkSegment  dds初始化使用的网段ip
 * @return int 成功返回1，失败返回0
 *
 * 该函数初始化DDS配置，创建参与者。
 */
int RealtimeNetMonitorCreatePart(int ddsDominId, RtNetMonitorHandle * handle);

/**
 * @brief 创建Topic和操作器（Reader/Writer）
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1，失败返回0
 *
 * 该函数为每个Topic创建对应的Reader或Writer，并将其存储到句柄中。
 */
int RealtimeNetMonitorCreateTopicAndOper(RtNetMonitorHandle * handle);

/**
 * @brief 创建条件变量并注册到WaitSet
 *
 * @param handle 句柄，包含Reader列表
 * @return int 成功返回1
 *
 * 该函数为每个Reader创建ReadCondition，并将其注册到WaitSet中。
 */
int RealtimeNetMonitorCreateCond(RtNetMonitorHandle *handle);

/**
 * @brief 创建消息接收线程
 *
 * @param handle 句柄，包含Reader列表
 * @return int 成功返回1
 *
 * 如果存在Reader，则创建一个线程用于接收消息。
 */
int RealtimeNetMonitorCreateThread(RtNetMonitorHandle *handle);

/**
 * @brief 消息接收线程，用于处理接收到的DDS消息
 *
 * @param inPara 输入参数，指向RtNetMonitorHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetMonitorRecvThread(void * inPara);

/**
 * @brief 处理心跳应答消息
 *
 * @param inPara 输入参数，指向RtNetMonitorHandle
 * @param msg 输入参数，指向应答消息
 * @return 
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
void RealtimeNetMonitorDoResMsg(RtNetMonitorHandle *handle, void * msg);

/**
 * @brief 处理节点应答消息
 *
 * @param inPara 输入参数，指向RtNetMonitorHandle
 * @param msg 输入参数，指向应答消息
 * @return
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
void RealtimeNetMonitorDoNodeMsg(RtNetMonitorHandle *handle, void * msg);

/**
 * @brief 处理业务数据消息
 *
 * @param inPara 输入参数，指向RtNetMonitorHandle
 * @param triggered_reader 消息对应的reader
 * @param msg 输入参数，指向应答消息
 * @return
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
void RealtimeNetMonitorDoDataMsg(RtNetMonitorHandle *handle, dds_attach_t triggered_reader, void * msg);

/**
 * @brief 消息发送线程，用于发送DDS消息
 *
 * @param inPara 输入参数，指向RtNetMonitorHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetMonitorSendThread(void * inPara);

/**
 * @brief 查找Topic
 *
 * @param handle 句柄
 * @param reader 哈希索引
 * @return int item
 *
 * 根据偏移量在哈希表中查找对应的Topic item。
 */
MoitorTopicItem * RealtimeNetMonitorFindTopicbyReader(RtNetMonitorHandle *handle, unsigned int reader);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_INNER_H
