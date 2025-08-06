#ifndef REALTIME_NET_MIDDLEWARE_MONITOR_H
#define REALTIME_NET_MIDDLEWARE_MONITOR_H

#include "dds/dds.h"
#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif
	
#pragma pack(1) /*1字节对齐*/

enum MONITOR_TOPIC_TYPE
{
	MONITOR_TOPIC_TYPE_READ = 1,
	MONITOR_TOPIC_TYPE_WRITE
};

typedef struct ST_MonitorTopicItem_tag
{
	char*		topicName;				// 通过DDS发布/订阅信息使用到的TopicName
	int			size;					// TopicName对应数据的缓冲区大小
	int			status;					// 该TopicName是读还是写，该字段主要为了兼容反射内存网
}MonitorTopicItem;

typedef struct ST_NodeInfo_tag
{
	const char*				networkSegment;		// ip
	int						processId;			// 进程ID
	int						threadId;			// 线程ID
	MonitorTopicItem*		topicMapList;		// topic信息列表
	int						topicMapListLength;	// 映射表的长度
}RtNodeInfo;


#pragma pack()/*还原默认对齐*/

// 定义函数指针，把节点信息传递给业务，需要业务把信息拷贝出去。
typedef void(*OnReceiverNodeFun)(RtNodeInfo * node);

// 定义函数指针，可以通过该回调接收已注册topicName的数据
typedef void(*OnReceiverHeartBeatFun)(char ip, int processId, int threadId);

// 定义函数指针，可以通过该回调接收已注册topicName的数据
typedef void(*OnReceiverDataFun)(char * topicName, void * pBuffer, unsigned int size, char ip, int processId, int threadId);

/**
 * @brief	监控的初始化
 *
 * 根据外部代码调用传来的参数，进行监控的初始化
 *
 * @param	ddsDominId		dds域id
 * @param	timeout			心跳的时间间隔
 * @param	onReceiverNode	节点信息的回调
 * @param	onReceiverHeartBeat	心跳信息的回调
 * @return					返回句柄
 */
DDS_EXPORT  void * MonitorInit(int ddsDominId, int timeout, OnReceiverNodeFun onReceiverNode, OnReceiverHeartBeatFun onReceiverHeartBeat, OnReceiverDataFun OnReceiverData);

/**
 * @brief	增加topic
 *
 * 根据增加的topic,监控所有该topic的数据，通过回调返回给业务层
 *
 * @param	pContext		初始化时，返回的句柄
 * @param	topicName		topic名
 * @return			函数调用结果 1-成功 0-失败
 */
DDS_EXPORT  int MonitorAddTopic(void* pContext, char * topicName);


/**
 * @brief	通信中间件的释放
 *
 *
 *
 * @param	pContext 初始化时，返回的句柄
 * @return	null
 */
DDS_EXPORT void  MonitortFree(void* pContext);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_H
