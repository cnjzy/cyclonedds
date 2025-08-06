#ifndef REALTIME_NET_MIDDLEWARE_INNER_H
#define REALTIME_NET_MIDDLEWARE_INNER_H

#include "dds/ddsrt/threads.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/time.h"

#include "RealtimeNetMiddleware.h"

#ifdef __cplusplus
extern "C" {
#endif		

#pragma pack(1) /*1字节对齐*/

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
}TopicMapInnerItem;

typedef struct ST_RtNetHandle_tag
{
	int					ddsDominId;			// dds域id
	char *				networkSegment;		// dds初始化使用的网段ip（当机器有多个网卡时，需要确保该网段的设备能收到dds信息），机器只有单个网卡时可为空
	TopicMapInnerItem *	topicMapList;		// dds注册的topic信息列表，因为要兼容反射内存网的offset，所以初始化时传入TopicName和Offset的映射表
	int					topicMapListLength;	// 映射表的长度
	OnReceiverDataFun	onReceiverTopic;	// 接收信息的回调，通过函数指针实现。外部传入初始化结构体时，可以传入该变量，这样外部可以通过回调异步接收数据。如果外部传空，
	int					timeout;			// 接收消息的超时时间，单位秒

	dds_entity_t		participant;		//
	dds_entity_t		waitset;			//

	ddsrt_thread_t		tid;				//内部接收消息的线程ID
	bool				exitFlg;			//退出标志
	struct ddsrt_hh *	typecache;			//存储item的哈希handle
	ddsrt_mutex_t		lock;				//内部数据拷贝锁
}RtNetHandle;


#pragma pack()/*还原默认对齐*/

void RealtimeNetSetParam(RtNetInitParam *initParam, RtNetHandle * handle);
int RealtimeNetInitHash(RtNetHandle *handle);
int RealtimeNetCreatePart(RtNetHandle *handle);
int RealtimeNetCreateTopicAndOper(RtNetHandle *handle);
int RealtimeNetCreateCond(RtNetHandle *handle);
int RealtimeNetMsgThread(void * inPara);
int RealtimeNetCreateThread(RtNetHandle *handle);
int RealtimeNetFindTopic(RtNetHandle *handle, unsigned int offset);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_INNER_H
