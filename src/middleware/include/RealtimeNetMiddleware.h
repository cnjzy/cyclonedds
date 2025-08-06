#ifndef REALTIME_NET_MIDDLEWARE_H
#define REALTIME_NET_MIDDLEWARE_H

#ifdef __cplusplus
extern "C" {
#endif


// 定义函数指针，可以通过该回调接收已注册topicName的数据
typedef void (*OnReceiverDataFun)(char* topicName,unsigned int offset,void* pBuffer,unsigned int size);

#pragma pack(1) /*1字节对齐*/

enum TOPIC_TYPE
{
	TOPIC_TYPE_READ = 1,
	TOPIC_TYPE_WRITE
};

typedef struct ST_TopicMapItem_tag
{
	char*		topicName;				// 通过DDS发布/订阅信息使用到的TopicName
	int			offset;					// 原反射内存网的Offset
	int			size;					// TopicName对应数据的缓冲区大小
	int			status;					// 该TopicName是读还是写，该字段主要为了兼容反射内存网
}TopicMapItem;

typedef struct ST_RtNetInitParam_tag
{
	int					ddsDominId;			// dds域id
	const char*			networkSegment;		// dds初始化使用的网段ip（当机器有多个网卡时，需要确保该网段的设备能收到dds信息），机器只有单个网卡时可为空
	TopicMapItem*		topicMapList;		// dds注册的topic信息列表，因为要兼容反射内存网的offset，所以初始化时传入TopicName和Offset的映射表
	int					topicMapListLength;	// 映射表的长度
	OnReceiverDataFun	onReceiverTopic;	// 接收信息的回调，通过函数指针实现。外部传入初始化结构体时，可以传入该变量，这样外部可以通过回调异步接收数据。如果外部传空，
	int					timeout;			// 接收消息的超时时间，单位秒
}RtNetInitParam;


#pragma pack()/*还原默认对齐*/


/**
 * @brief	通信中间件的初始化
 *
 * 根据外部代码调用传来的参数，进行中间件的初始化
 *
 * @param	initParam 外部传递的初始化参数结构体
 * @return	DDS内部库里返回的对象指针，后续在调用RealtimeNetRead和RealtimeNetWrite方法时使用
 */
void*	RealtimeNetInit(RtNetInitParam initParam);

/**
 * @brief	读取数据
 *
 * 根据初始化时返回的对象，以及offset值进行数据的读取
 *
 * @param pContext	调用RealtimeInit时返回的对象指针
 * @param offset	原反射内存网使用的offset，库里需要根据TopicName和Offset的映射关系，读取TopicName接收到的数据
 * @param pBuffer	接收数据缓冲区
 * @param pBuffer	接收数据缓冲长度
 * @return			函数调用结果 1-成功 0-失败
 */
int		RealtimeNetRead(void* pContext, unsigned int offset, void* pBuffer, unsigned int size);

/**
 * @brief	写入数据
 *
 * 根据初始化时返回的对象，以及offset值进行数据的写入
 *
 * @param pContext	调用RealtimeInit时返回的对象指针
 * @param offset	原反射内存网使用的offset，库里需要根据TopicName和Offset的映射关系，往TopicName发布数据
 * @param pBuffer	接收数据缓冲区
 * @param pBuffer	接收数据缓冲长度
 * @return			函数调用结果 1-成功 0-失败
 */
int		RealtimeNetWrite(void* pContext,unsigned int offset,void* pBuffer,unsigned int size);

/**
 * @brief	通信中间件的释放
 *
 * 
 *
 * @param	null
 * @return	null
 */
void  RealtimeNetFree();

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_H
