#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#else
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#endif


#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#else
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "dds/dds.h"
#include "dds/ddsi/ddsi_xt_typeinfo.h"
#include "dds/ddsi/ddsi_config.h"
#include "dds/ddsrt/hopscotch.h"

#include "dds/ddsi/RealtimeNetMiddleware.h"
#include "dds/ddsi/RealtimeNetMiddleware_inner.h"
#include "dds/ddsi/RealtimeNetMsg.h"
#include "dds/ddsi/RealtimeNetLicense.h"
#include "dds/ddsi/RealtimeNetMonitor_inner.h"
#include "dds/ddsi/RealtimeNetMontorData.h"

#define MAX_SAMPLES 1	// 定义最大样本数，用于限制一次读取的最大数据样本数量

/**
 * @brief 计算哈希值，用于通过offset存储和查找TopicMapInnerItem
 *
 * @param vinfo 指向TopicMapInnerItem的指针
 * @return uint32_t 计算出的哈希值
 *
 * 该函数通过偏移量和两个常量计算哈希值，确保哈希表的分布均匀。
 */
static uint32_t typecache_hash (const void *vinfo)
{
	const TopicMapInnerItem *info = vinfo;
	// 使用偏移量和常量计算哈希值
	return (uint32_t) (((info->offset + UINT64_C (16292676669999574021)) * UINT64_C (10242350189706880077)) >> 32);   
}


/**
 * @brief 计算哈希值，用于通过reader存储和查找TopicMapInnerItem
 *
 * @param vinfo 指向TopicMapInnerItem的指针
 * @return uint32_t 计算出的哈希值
 *
 * 该函数通过偏移量和两个常量计算哈希值，确保哈希表的分布均匀。
 */
static uint32_t typecachereader_hash(const void *vinfo)
{
	const TopicMapInnerItem *info = vinfo;
	// 使用偏移量和常量计算哈希值
	return (uint32_t)(((info->operator + UINT64_C(16292676669999574021)) * UINT64_C(10242350189706880077)) >> 32);
}

/**
 * @brief 通过offset比较两个TopicMapInnerItem是否相等
 *
 * @param va 指向第一个TopicMapInnerItem的指针
 * @param vb 指向第二个TopicMapInnerItem的指针
 * @return int 如果相等返回1，否则返回0
 *
 * 该函数通过比较两个TopicMapInnerItem的`offset`字段来判断它们是否相等。
 */
static int typecache_equal (const void *va, const void *vb)
{
  const TopicMapInnerItem *a = va;
  const TopicMapInnerItem *b = vb;
  return a->offset == b->offset;
}


/**
 * @brief 通过reader比较两个TopicMapInnerItem是否相等
 *
 * @param va 指向第一个TopicMapInnerItem的指针
 * @param vb 指向第二个TopicMapInnerItem的指针
 * @return int 如果相等返回1，否则返回0
 *
 * 该函数通过比较两个TopicMapInnerItem的`offset`字段来判断它们是否相等。
 */
static int typecachereader_equal(const void *va, const void *vb)
{
	const TopicMapInnerItem *a = va;
	const TopicMapInnerItem *b = vb;
	return a->operator == b->operator;
}

/**
 * @brief 初始化网络参数并设置到句柄中
 *
 * @param initParam 初始化参数，包含网络段、Topic映射列表等信息
 * @param handle 句柄，用于存储初始化后的参数
 *
 * 该函数将`initParam`中的参数复制到`handle`中，并分配必要的内存。
 * 如果未设置超时时间，则默认设置为1小时。
 */
void RealtimeNetSetParam(RtNetInitParam *initParam, RtNetHandle * handle)
{
	// 设置DDS域ID
	handle->ddsDominId = initParam->ddsDominId;


	// 复制网络段信息
	size_t length = 0;
	if (NULL != initParam->networkSegment)
	{
		length = strlen(initParam->networkSegment);
	}
	handle->networkSegment = malloc(length + 1);
	memcpy(handle->networkSegment, initParam->networkSegment, length);
	handle->networkSegment[length] = '\0';

	// 初始化Topic映射列表
	handle->topicMapListLength = initParam->topicMapListLength;
	TopicMapInnerItem *desList = malloc(sizeof(TopicMapInnerItem) * handle->topicMapListLength);
	memset(desList, 0, sizeof(TopicMapInnerItem) * handle->topicMapListLength);
	TopicMapItem *srcList = initParam->topicMapList;
	for (int i = 0; i < handle->topicMapListLength; i++) {
		OS_LOG_INFO(handle->log->pad_log_handle_fp, "Mapping: Offset=%u, TopicName=%s", srcList[i].offset, srcList[i].topicName);
		// 复制Topic名称
		length = strlen(srcList[i].topicName);
		desList[i].topicName = malloc(length + 1);
		memset(desList[i].topicName, 0, length + 1);
		memcpy(desList[i].topicName, srcList[i].topicName, length);
		//strncpy(desList[i].topicName, srcList[i].topicName, length);

		// 复制其他字段
		desList[i].offset = srcList[i].offset;
		desList[i].size = srcList[i].size;
		desList[i].status = srcList[i].status;
		desList[i].buf = NULL;

		// 如果是读取类型的Topic，分配缓冲区
		if (TOPIC_TYPE_READ == desList[i].status) {
			desList[i].buf = malloc(desList[i].size + 1);
			memset(desList[i].buf, 0, desList[i].size + 1);
		}
		desList[i].num = i;		// 设置Topic的索引
	}
	handle->topicMapList = desList;

	// 设置回调函数和超时时间
	handle->onReceiverTopic = initParam->onReceiverTopic;

	//设置监控相关的信息
#ifdef _WIN32
	handle->processId = GetCurrentProcessId();	//windows
	handle->threadId = GetCurrentThreadId();
#else
	handle->processId = getpid(); // linux
	handle->threadId = pthread_self() ;
#endif
	if (0 == strlen(handle->networkSegment))
	{
		get_local_ips(handle->ipAddr);
	}
	else
	{
		strncpy(handle->ipAddr, initParam->networkSegment, sizeof(handle->ipAddr)-1);
	}
	printf("ip[%s]\n", handle->ipAddr);
	handle->ip = getIpLastChar(handle->ipAddr);

	// 初始化退出标志和互斥锁
	handle->exitFlg = false;
	ddsrt_mutex_init(&handle->lock);
}

/**
 * @brief 初始化哈希表，用于存储TopicMapInnerItem
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1
 *
 * 该函数创建一个哈希表，并将Topic映射列表中的每个元素添加到哈希表中。
 */
int RealtimeNetInitHash(RtNetHandle *handle)
{
	// 创建offset read哈希表，初始大小为32
	handle->offsetReadHash = ddsrt_hh_new (32, typecache_hash, typecache_equal);
	// 创建offset write哈希表，初始大小为32
	handle->offsetWriteHash = ddsrt_hh_new(32, typecache_hash, typecache_equal);

	// 将Topic映射列表中的每个元素添加到哈希表中
	for (int i = 0; i < handle->topicMapListLength; i++) {
		if (TOPIC_TYPE_READ == handle->topicMapList[i].status)
		{
			ddsrt_hh_add(handle->offsetReadHash, &handle->topicMapList[i]);
		}
		else
		{
			ddsrt_hh_add(handle->offsetWriteHash, &handle->topicMapList[i]);
		}

	}

	// 创建reader哈希表，初始大小为32
	handle->readerHash = ddsrt_hh_new(32, typecachereader_hash, typecachereader_equal);

	// 将Topic映射列表中的每个元素添加到哈希表中
	for (int i = 0; i < handle->topicMapListLength; i++) {
		ddsrt_hh_add(handle->readerHash, &handle->topicMapList[i]);
		//printf("ddsrt_hh_add: operator[%d]\n", handle->topicMapList[i].operator);
	}

	return 1;
}

/**
 * @brief 创建DDS参与者
 *
 * @param handle 句柄，包含网络段信息
 * @param myQos  Qos
 * @return int 成功返回1，失败返回0
 *
 * 该函数初始化DDS配置，创建参与者，并将其存储到句柄中。
 */
int RealtimeNetCreatePart(RtNetHandle *handle, Qos * myQos)
{
	// 初始化默认配置
	ddsi_config_init_default(&handle->raw);
	handle->raw.depr_networkAddressString = handle->networkSegment;
	
	// 创建DDS域
	dds_create_domain_with_rawconfig(handle->ddsDominId, &handle->raw);

	//创建qos
	handle->qos = dds_create_qos();
	//设置qos
	RealtimeNetReadConf_setQos(handle->qos, myQos);

	// 创建参与者
	handle->participant = dds_create_participant(handle->ddsDominId, handle->qos, NULL);
	if (handle->participant < 0) {
		DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-handle->participant));
		return 0;
	}
	


	return 1;
}

/**
 * @brief 创建Topic和操作器（Reader/Writer）
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1，失败返回0
 *
 * 该函数为每个Topic创建对应的Reader或Writer，并将其存储到句柄中。
 */
int RealtimeNetCreateTopicAndOper(RtNetHandle *handle)
{
	int ret = 0;
	// 创建业务相关的Topic和操作器
	ret = RealtimeNetCreateNormalTopicAndOper(handle);
	if (0 == ret)
	{
		OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetCreateNormalTopicAndOper: call RealtimeNetCreateNormalTopicAndOper err. ");
		return ret;
	}

	// 创建监控相关的Topic和操作器
	ret = RealtimeNetCreatMonitorTopicAndOper(handle);
	if (0 == ret)
	{
		OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetCreateNormalTopicAndOper: call RealtimeNetCreatMonitorTopicAndOper err. ");
		return ret;
	}

	return 1;
}
/**
 * @brief 创建Topic和操作器（Reader/Writer）
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1，失败返回0
 *
 * 该函数为每个Topic创建对应的Reader或Writer，并将其存储到句柄中。
 */
int RealtimeNetCreateNormalTopicAndOper(RtNetHandle *handle)
{
	//printf("===   call  RealtimeNetCreateNormalTopicAndOper\n");
	int ret = 0;

	// 创建QoS配置，设置为最佳努力可靠性
	/* Create a reliable Reader. */
	//dds_qos_t *qos;
	//qos = dds_create_qos();
	//dds_qset_reliability(qos, DDS_RELIABILITY_BEST_EFFORT, 0);

	// 遍历Topic映射列表，为每个Topic创建Reader或Writer
	for (int i = 0; i < handle->topicMapListLength; i++) {
		/* Create a Topic. */
		// 创建Topic
		dds_entity_t * topic = &(handle->topicMapList[i].topic);
		char * topicName = handle->topicMapList[i].topicName;
		*topic = dds_create_topic(
			handle->participant, &RealtimeNetData_Msg_desc, topicName, handle->qos, NULL);
		if (*topic < 0)
		{
			DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-*topic));
			ret = 0;
			goto error;
		}

		// 根据Topic类型创建Writer或Reader
		if (TOPIC_TYPE_WRITE == handle->topicMapList[i].status) {
			/* Create a Writer. */
			dds_entity_t * writer = &(handle->topicMapList[i].operator);
			*writer = dds_create_writer(handle->participant, *topic, handle->qos, NULL);
			if (*writer < 0) {
				DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-*writer));
				ret = 0;
				goto error;
			}
		}
		else {
			/* Create a reader. */
			dds_entity_t * reader = &(handle->topicMapList[i].operator);
			*reader = dds_create_reader(handle->participant, *topic, handle->qos, NULL);
			if (*reader < 0) {
				DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-*reader));
				ret = 0;
				goto error;
			}
			else {
				//printf("create reader ok.reader:%d\n", *reader);
			}
		}
	}

	ret = 1;

error:
	// 删除QoS配置
	//dds_delete_qos(qos);
	return ret;
}

/**
 * @brief 创建监控相关的Topic和操作器（Reader/Writer）
 *
 * @param handle 句柄，包含Topic映射列表
 * @return int 成功返回1，失败返回0
 *
 * 该函数为每个Topic创建对应的Reader或Writer，并将其存储到句柄中。
 */
int RealtimeNetCreatMonitorTopicAndOper(RtNetHandle *handle)
{
	int ret = 0;
	
	//创建心跳请求的topic
	handle->topicReq = dds_create_topic(
		handle->participant, &RealtimeNetMonitor_Req_desc, c_topicNameReq, NULL, NULL);
	if (handle->topicReq < 0)
	{
		DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(handle->topicReq));
		return 0;
	}
	else {
		//printf("create topic topicReq ok.topicReq:%d\n", handle->topicReq);
	}


	//创建心跳请求的reader
	handle->readerReq = dds_create_reader(handle->participant, handle->topicReq, NULL, NULL);
	if (handle->readerReq < 0) {
		DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-(handle->readerReq)));
		return 0;
	}
	else {
		//printf("create Writer writerReq ok.Writer:%d\n", handle->readerReq);
	}
	
	
	//创建心跳应答的topic
	handle->topicRes = dds_create_topic(
		handle->participant, &RealtimeNetMonitor_Res_desc, c_topicNameRes, NULL, NULL);
	if (handle->topicRes < 0)
	{
		DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(handle->topicRes));
		return 0;
	}
	else {
		//printf("create topic topicReq ok.topicReq:%d\n", handle->topicRes);
	}

	//创建心跳应答的writer
	handle->writerRes = dds_create_writer(handle->participant, handle->topicRes, NULL, NULL);
	if (handle->writerRes < 0) {
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-(handle->writerRes)));
		return 0;
	}
	else {
		//printf("create reader readerRes ok.reader:%d\n", handle->writerRes);
	}


	//创建节点应答的topic
	handle->topicNode = dds_create_topic(
		handle->participant, &RealtimeNetMonitor_Node_desc, c_topicNameNode, NULL, NULL);
	if (handle->topicNode < 0)
	{
		DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(handle->topicNode));
		return 0;
	}
	else {
		//printf("create topic topicReq ok.topicReq:%d\n", handle->topicNode);
	}

	//创建节点应答的reader
	handle->writerNode = dds_create_writer(handle->participant, handle->topicNode, NULL, NULL);
	if (handle->writerNode < 0) {
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-(handle->writerNode)));
		return 0;
	}
	else {
		//printf("create reader readerRes ok.reader:%d\n", handle->writerNode);
	}
	
	return 1;
}

/**
 * @brief 创建条件变量并注册到WaitSet
 *
 * @param handle 句柄，包含Reader列表
 * @return int 成功返回1
 *
 * 该函数为每个Reader创建ReadCondition，并将其注册到WaitSet中。
 */
int RealtimeNetCreateCond(RtNetHandle *handle)
{
	//dds_entity_t waitset;
	dds_entity_t reader;
	dds_entity_t readcond;

	// 创建WaitSet
	handle->waitset = dds_create_waitset(handle->participant);
	//所有reader注册条件触发
	for (int i = 0; i < handle->topicMapListLength; i++) {
		if(TOPIC_TYPE_READ == handle->topicMapList[i].status) {
			reader = handle->topicMapList[i].operator;
			readcond = dds_create_readcondition(reader, DDS_ANY_STATE);
			dds_waitset_attach(handle->waitset, readcond, reader);
		}
	}

	//监控的reader注册条件触发
	readcond = dds_create_readcondition(handle->readerReq, DDS_ANY_STATE);
	dds_waitset_attach(handle->waitset, readcond, handle->readerReq);

	return 1;
}

/**
 * @brief 消息接收线程，用于处理接收到的DDS消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetMsgThread(void * inPara)
{
	RtNetHandle *handle = inPara;
	//printf("new thread ok\n");
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "new thread ok");
	dds_sleepfor(DDS_SECS(3));


	int ret;
	dds_attach_t triggered_reader;
	while (!handle->exitFlg) {
		//wl_add  相对时间暂时定位1秒
		dds_time_t tstop = dds_time() + DDS_SECS(1);
		triggered_reader = 0;

		// 等待WaitSet触发
		ret = dds_waitset_wait_until(handle->waitset, &triggered_reader, 1, tstop);
		if (ret < 0) {
			printf("dds_waitset_wait_until ret err: %d\n", ret);
			OS_LOG_INFO(handle->log->pad_log_handle_fp, "dds_waitset_wait_until ret err:");
			fflush(stdout);
			continue;
		}
		else if (0 == ret) {
			//printf("dds_waitset_wait_until timeout . do nothing\n");
			//OS_LOG_INFO(handle->log->pad_log_handle_fp, "dds_waitset_wait_until timeout . do nothing");
			//fflush(stdout);
			continue;
		}
		else {
			//printf("dds_waitset_wait_until ok。 ret : %d\n", ret);
			//OS_LOG_INFO(handle->log->pad_log_handle_fp, "dds_waitset_wait_until ok。 ret : %d", ret);
			//fflush(stdout);
		}

		// 读取消息
		void *samples[MAX_SAMPLES] = { NULL };
		dds_sample_info_t infos[MAX_SAMPLES];
		ret = dds_take((dds_entity_t)triggered_reader, samples, infos, 1, 1);
		if (ret <= 0)
		{
			printf("dds_take ret <=0: %d。 continue run\n", ret);
			fflush(stdout);
			//dds_sleepfor(DDS_SECS(10));
			continue;
		}

		// 处理有效数据
		//printf("dds_take return ok: %d\n", ret);
		if ((infos[0].valid_data) && (NULL != samples[0])) {
			if (triggered_reader == handle->readerReq) {
				RealtimeNetDoMonitalMsg(handle, samples[0]);
			}
			else
			{
				RealtimeNetDoNormalMsg(handle, triggered_reader, samples[0]);
			}
			
			ret = dds_return_loan((dds_entity_t)triggered_reader, samples, 1);
		}
		else {
			printf("valid_data is false\n");
		}

	}

	printf("recv msg thread finish\n");
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "recv msg thread exit");
	fflush(stdout);
	handle->recvThreadExitFlg = true;
	return 1;
}

/**
 * @brief 创建消息接收线程
 *
 * @param handle 句柄，包含Reader列表
 * @return int 成功返回1
 *
 * 如果存在Reader，则创建一个线程用于接收消息。
 */
int RealtimeNetCreateThread(RtNetHandle *handle)
{
	//支持监控功能后，所有dds库均需要创建接收线程。
	/*
	bool haveReader = false;

	//所有reader注册条件触发
	for (int i = 0; i < handle->topicMapListLength; i++) {
		if (TOPIC_TYPE_READ == handle->topicMapList[i].status) {
			haveReader = true;
			break;
		}
	}

	// 如果没有Reader，直接返回
	if (!haveReader)
		return 1;
	*/

	// 创建线程
	dds_return_t rc;
	ddsrt_threadattr_t tattr;
	ddsrt_threadattr_init(&tattr);
	rc = ddsrt_thread_create(&(handle->tid), "recv", &tattr, RealtimeNetMsgThread, handle);	
	return 1;
}

/**
 * @brief 初始化实时网络
 *
 * @param initParam 初始化参数
 * @return void* 成功返回句柄，失败返回NULL
 *
 * 该函数依次调用各个初始化函数，完成实时网络的初始化。
 */
void * RealtimeNetInit(RtNetInitParam initParam)
{
	printf("===   call  RealtimeNetInit\n");
	RtNetHandle *handle = malloc(sizeof(RtNetHandle));
	memset(handle, 0, sizeof(RtNetHandle));

	//RealtimeNetConf * pConf;
	char * filePath = "RealtimeNetMiddleware.xml";
	handle->pConf = RealtimeNetReadConf_malloc();
	RealtimeNetReadConf(filePath, handle->pConf);

	long long start = os_getCurrentEpochTime();

	// 初始化日志
	LogInit(handle, handle->pConf->log.logLevel);
	//等待日志系统启动完毕
	dds_sleepfor(DDS_MSECS(30));

	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetInit: Start initialization");
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "Parameters: ddsDominId=%d, networkSegment=%s, topicMapListLength=%d",
		initParam.ddsDominId, initParam.networkSegment, initParam.topicMapListLength);

	int ret = 0;

/*  //鉴权代码临时注释掉，待后续打开
	//鉴权
	ret = RealtimeNetAuthentication(pConf->lice.licencePath);
	if (!ret) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetAuthentication failed");
		return NULL;
	}
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetAuthentication completed");
*/


	// 设置参数
	/*inputpara to handle */
	RealtimeNetSetParam(&initParam, handle);
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetSetParam completed");


	// 创建参与者
	  /* Create a Participant. */
	ret = RealtimeNetCreatePart(handle, &(handle->pConf->qos));
	if (0 == ret) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetCreatePart failed");
		return NULL;
	}
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetCreatePart completed");
	
	// 创建Topic和操作器
	/* create topic and operator */
	ret = RealtimeNetCreateTopicAndOper(handle);
	if (0 == ret)	{
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetCreateTopicAndOper failed");
		return NULL;
	}
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetCreateTopicAndOper completed");
	   

	// 初始化哈希表
	RealtimeNetInitHash(handle);
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetInitHash completed");

	// 创建条件变量
	/* create cond */
	ret = RealtimeNetCreateCond(handle);
	if (0 == ret) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetCreateCond failed");
		return NULL;
	}
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetCreateCond completed");

	// 创建消息接收线程
	//create thread read msg
	RealtimeNetCreateThread(handle);
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetInit: Initialization completed successfully");

	long long end = os_getCurrentEpochTime();
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetInit Operation took %f seconds", (double)(end - start) / 1000);


	//RealtimeNetReadConf_free(pConf);

	//防止虚拟机等配置不好的机器初始化未完成，延时一会儿。
	dds_sleepfor(DDS_MSECS(100));

	//发送一条监控消息
	RealtimeNetSendMonitalNodeMsg(handle);
	return handle;
}

/**
 * @brief 读取数据
 *
 * @param pContext 上下文句柄
 * @param offset 偏移量
 * @param pBuffer 缓冲区
 * @param size 数据大小
 * @return int 成功返回1，失败返回0
 *
 * 根据偏移量查找对应的Topic，并将数据复制到缓冲区中。
 */
int	RealtimeNetRead(void* pContext, unsigned int offset, void* pBuffer, unsigned int size)
{
	RtNetHandle *handle;
	handle = pContext;
	int ret = 0;
	long long start = os_getCurrentEpochTime();
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetRead: Start reading data. Offset=%u, Size=%u", offset, size);
	//printf("===   call  RealtimeNetRead\n");

	if (0 == size)
	{
		return 0;
	}

	// 查找Topic
	int num = RealtimeNetFindTopicbyReadOffset(handle, offset);
	if (num < 0) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetRead: Invalid offset=%u. Topic not found.", offset);
		printf("offset[%d] not found topic. read error.\n", offset);
		return 0;
	}

	// 检查数据大小是否匹配
	if (size != handle->topicMapList[num].size) {
		printf("size is error. inputsize[%d], topicsize[%d]\n", size, handle->topicMapList[num].size);
		OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetRead: Size mismatch. InputSize=%u, TopicSize=%u", size, handle->topicMapList[num].size);
		return 0;
	}

	//如果有数据，再提供
	if (handle->topicMapList[num].haveDataFlg)
	{
		ret = 1;
		// 复制数据到缓冲区
		ddsrt_mutex_lock(&handle->lock);
		OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "Acquired lock");

		memcpy(pBuffer, handle->topicMapList[num].buf, size);

		ddsrt_mutex_unlock(&handle->lock);
		OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "Released lock");
		OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetRead: Successfully read data. Offset=%u, Size=%u", offset, size);
	}

	long long end = os_getCurrentEpochTime();
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetRead Operation took %f seconds", (double)(end - start) / 1000);
	return ret;
}

/**
 * @brief 查找Topic
 *
 * @param handle 句柄
 * @param offset 偏移量
 * @return int 成功返回Topic索引，失败返回-1
 *
 * 根据偏移量在哈希表中查找对应的Topic。
 */
int RealtimeNetFindTopicbyReadOffset(RtNetHandle *handle, unsigned int offset)
{
	TopicMapInnerItem inItem;
	inItem.offset = offset;
	TopicMapInnerItem * pOutItem;

	// 在哈希表中查找
	pOutItem = ddsrt_hh_lookup (handle->offsetReadHash, &inItem);
	if (NULL == pOutItem) {
		return -1;
	}
	//printf("lookup result. offset[%d], topicname[%s], num[%d]\n", offset, pOutItem->topicName, pOutItem->num);

	return pOutItem->num;

}

/**
 * @brief 查找Topic
 *
 * @param handle 句柄
 * @param offset 偏移量
 * @return int 成功返回Topic索引，失败返回-1
 *
 * 根据偏移量在哈希表中查找对应的Topic。
 */
int RealtimeNetFindTopicbyWriteOffset(RtNetHandle *handle, unsigned int offset)
{
	TopicMapInnerItem inItem;
	inItem.offset = offset;
	TopicMapInnerItem * pOutItem;

	// 在哈希表中查找
	pOutItem = ddsrt_hh_lookup(handle->offsetWriteHash, &inItem);
	if (NULL == pOutItem) {
		return -1;
	}
	//printf("lookup result. offset[%d], topicname[%s], num[%d]\n", offset, pOutItem->topicName, pOutItem->num);

	return pOutItem->num;

}

/**
 * @brief 查找Topic
 *
 * @param handle 句柄
 * @param reader 哈希索引
 * @return int 成功返回Topic索引，失败返回-1
 *
 * 根据偏移量在哈希表中查找对应的Topic。
 */
int RealtimeNetFindTopicbyReader(RtNetHandle *handle, unsigned int reader)
{
	TopicMapInnerItem inItem;
	inItem.operator = reader;
	TopicMapInnerItem * pOutItem;
	//printf("RealtimeNetFindTopicbyReader: reader[%d]\n", reader);

	// 在哈希表中查找
	pOutItem = ddsrt_hh_lookup(handle->readerHash, &inItem);
	if (NULL == pOutItem) {
		return -1;
	}
	//printf("lookup result. reader[%d], topicname[%s], num[%d]\n", reader, pOutItem->topicName, pOutItem->num);

	return pOutItem->num;

}

/**
 * @brief 初始化日志
 *
 * @param handle 句柄
 * @return int 成功返回0
 *
 * 该函数用于初始化日志系统，当前实现为空。
 */
int LogInit(RtNetHandle *handle, os_log_level level)
{

	/* 初始化日志模块 */
	handle->log = malloc(sizeof(LogSetting));
	memset(handle->log, 0, sizeof(LogSetting));
	handle->log->log_attr.level = level;
	if (0 != strlen(handle->pConf->log.logPath))
	{
		strncpy(handle->log->log_attr.path, handle->pConf->log.logPath, sizeof(handle->log->log_attr.path));
	}


	os_log_init("DDS", &handle->log->log_attr, &(handle->log->pad_log_handle_fp));

	OS_LOG_INFO(handle->log->pad_log_handle_fp, "Init log success\n");
	return 0;
}

/**
 * @brief 写入数据
 *
 * @param pContext 上下文句柄
 * @param offset 偏移量
 * @param pBuffer 数据缓冲区
 * @param size 数据大小
 * @return int 成功返回1，失败返回0
 *
 * 根据偏移量查找对应的Topic，并将数据写入。
 */
int		RealtimeNetWrite(void* pContext, unsigned int offset, void* pBuffer, unsigned int size)
{

	RtNetHandle *handle;
	RealtimeNetData_Msg msg;
	handle = pContext;
	long long start = os_getCurrentEpochTime();
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetWrite: Start writing data. Offset=%u, Size=%u", offset, size);

	//printf("===   call  RealtimeNetWrite\n");
	if (0 == size)
	{
		return 0;
	}

	// 查找Topic
	int num = RealtimeNetFindTopicbyWriteOffset(handle, offset);
	if (num < 0) {
		printf("offset[%d] not found topic. write error.\n", offset);
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetWrite: Invalid offset=%u. Topic not found.", offset);
		return 0;
	}

	// 检查数据大小是否匹配
	if (size != handle->topicMapList[num].size) {
		printf("input size is error. input size[%d], topic size[%d]\n", size, handle->topicMapList[num].size);
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetWrite: Size mismatch. InputSize=%u, TopicSize=%u", size, handle->topicMapList[num].size);
		return 0;

	}

	// 创建消息并写入
	/* Create a message to write. */
	//msg.offset = offset;
	msg.ip = handle->ip;
	msg.processId = handle->processId;
	msg.threadId = handle->threadId;
	msg.data._maximum = size;
	msg.data._length = size;
	msg.data._buffer = pBuffer;
	msg.data._release = false;

	dds_return_t rc;;
	rc = dds_write(handle->topicMapList[num].operator, &msg);
	if (rc != DDS_RETCODE_OK) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetWrite: Failed to write data. Offset=%u, Error=%s", offset, dds_strretcode(-rc));
		return 0;
	}

	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetWrite: Successfully write data. Offset=%u, Size=%u", offset, size);

	long long end = os_getCurrentEpochTime();
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetWrite Operation took %f seconds", (double)(end - start) / 1000);
	return 1;
}

/**
 * @brief 释放资源
 *
 * @param pContext 上下文句柄
 *
 * 释放句柄中分配的所有资源，包括内存、线程、互斥锁等。
 */
void  RealtimeNetFree(void* pContext)
{
	long long start = os_getCurrentEpochTime();
	RtNetHandle *handle;
	handle = pContext;

	// 设置退出标志并等待线程退出
	handle->exitFlg = true;
	while (!handle->recvThreadExitFlg)
	{
		dds_sleepfor(DDS_MSECS(100));
	}
	printf("接收线程退出，主程序也退出！\n");
	fflush(stdout);

	// 释放QOS配置
	RealtimeNetReadConf_free(handle->pConf);
	//释放qos
	dds_delete_qos(handle->qos);

	// 销毁互斥锁
	ddsrt_mutex_destroy(&handle->lock);
	printf("===  call  RealtimeNetFree\n");

	// 释放哈希表
	ddsrt_hh_free(handle->offsetReadHash);
	ddsrt_hh_free(handle->offsetWriteHash);
	ddsrt_hh_free(handle->readerHash);

	// 删除参与者
	dds_delete(handle->participant);

	// 释放Topic映射列表中的资源
	TopicMapInnerItem *desList = handle->topicMapList;
	for (int i = 0; i < handle->topicMapListLength; i++) {
		free(desList[i].topicName);
		if (TOPIC_TYPE_READ == desList[i].status) {
			free(desList[i].buf);
		}
	}	
	free(handle->topicMapList);	

	// 释放网络段信息和句柄
	free(handle->networkSegment);


	long long end = os_getCurrentEpochTime();
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetFree Operation took %f seconds", (double)(end - start) / 1000);
	
	// 释放日志和整个句柄资源
	free(handle->log);
	free(handle);
}
/**
 * @brief 设置日志级别
 *
 * @param handle 句柄
 * @param level 日志级别
 * @return int 成功返回1，失败返回0
 *
 * 该函数设置日志模块的默认级别。
 */
int setLogLevel(RtNetHandle *handle, os_log_level level)
{
	if (!handle->log)
	{
		return 0;
	}

	handle->log->pad_log_handle_fp->module_defualt_level = level;

	return 1;
}

/**
 * @brief	设置日志文件
 *
 * @param handle 句柄
 * @param	filePath	日志级别
 * @param	fileName	日志级别
 * @param	fileMaxSize	日志文件最大大小（字节）
 * @return	函数调用结果 1-成功 0-失败
 */
int setLogFile(RtNetHandle *handle, char filePath[256], char fileName[256], int fileMaxSize)
{
	if (!handle->log)
	{
		return 0;
	}
	// 设置日志路径
	strncpy(handle->log->pad_log_handle_fp->path, filePath, sizeof(handle->log->pad_log_handle_fp->path) - 1);
	handle->log->pad_log_handle_fp->path[sizeof(handle->log->pad_log_handle_fp->path) - 1] = '\0'; // Ensure null-termination 
	// 设置日志文件名
	strncpy(handle->log->pad_log_handle_fp->log_file_name, fileName, sizeof(handle->log->pad_log_handle_fp->log_file_name) - 1);
	handle->log->pad_log_handle_fp->log_file_name[sizeof(handle->log->pad_log_handle_fp->log_file_name) - 1] = '\0'; // Ensure null-termination 
	// 设置日志文件最大大小
	handle->log->pad_log_handle_fp->max_file_size = fileMaxSize;

	return 1;
}

/**
 * @brief	鉴权
 * 通过读取本地的lic文件，检测机器码是否正确，有效期是否正确
 * @return	函数调用结果 1-成功 0-失败
 */
int RealtimeNetAuthentication(char * filePath)
{
	int ret = 0;

	//如果文件为空，则鉴权失败
	if (NULL == filePath) {
		return 0;
	}

	char buf[32+1];
	memset(buf, 0, sizeof(buf));
	ret = RealtimeNetReadFile(filePath, buf, 32);
	if (0 == ret) {
		return 0;
	}

	ret = rnml_startVerify(buf);

	return ret;
}




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
int  RealtimeNetReadFile(char * fname, char *buf, int bufLen)
{
	size_t sz;
	FILE *fp = NULL;
	char *document = NULL;
	/* Get size if it is a accessible regular file (no dir or link). */
	sz = ac_regular_file_size(fname);
	if (sz > 0)
	{
		/* Open the actual file. */
		fp = fopen(fname, "rb");
		if (fp)
		{
			/* Read the content. */
			 fread(buf, 1, bufLen, fp);
			(void)fclose(fp);
			return 1;
		}
	}
	return 0;
}


/**
 * @brief 处理接收到的正常DDS消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetDoNormalMsg(RtNetHandle *handle, int reader, void * inMsg)
{
	int ret = 0;
	RealtimeNetData_Msg *msg = inMsg;
	// 查找对应的Topic
	int num = RealtimeNetFindTopicbyReader(handle, reader);
	if (num < 0) {
		printf("reader[%d] not found topic. read error.\n", reader);
		return 0;
	}
	TopicMapInnerItem *item = &handle->topicMapList[num];
	if (item->size != msg->data._length) {
		printf("recv msg length is err. length[%d], topic size[%d]\n", msg->data._length, item->size);
		return 0;
	}

	// 更新缓冲区并调用回调函数
	ddsrt_mutex_lock(&handle->lock);
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "Acquired lock");
	memcpy(item->buf, msg->data._buffer, item->size);
	ddsrt_mutex_unlock(&handle->lock);
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "Released lock");

	item->haveDataFlg = true;

	//printf("dds_take:Message (%d, %s)\n", msg->offset, item->buf);
	if (handle->onReceiverTopic) {
		OS_LOG_INFO(handle->log->pad_log_handle_fp, "OnReceiverDataFun: Callback triggered. TopicName=%s, Offset=%u,DataSize=%u",
			item->topicName, item->offset, msg->data._length);
		handle->onReceiverTopic(item->topicName, item->offset, msg->data._buffer, msg->data._length);
	}
	return 1;
}

/**
 * @brief 处理接收到的监控DDS消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetDoMonitalMsg(RtNetHandle *handle, void * msg)
{
	//printf("call RealtimeNetDoMonitalMsg\n");
	RealtimeNetMonitor_Req * req = msg;
	printf("call RealtimeNetDoMonitalMsg: req[%d]\n", req->seq);

	if (handle->seq == req->seq)
	{
		//printf("(handle->seq == req->seq)\n");
		RealtimeNetSendMonitalResMsg(handle);
	}
	else 
	{
		//printf("(handle->seq != req->seq)\n");
		handle->seq = req->seq;
		// call node msg
		RealtimeNetSendMonitalNodeMsg(handle);
	}
	return 1;
}

/**
 * @brief 发送监测的心跳应答消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 
 */
int RealtimeNetSendMonitalResMsg(RtNetHandle *handle)
{
	RealtimeNetMonitor_Res res;
	memset(&res, 0, sizeof(RealtimeNetMonitor_Res));
	res.processId = handle->processId;
	res.threadId = handle->threadId;
	res.ip = handle->ip;

	int rc = 0;
	rc = dds_write(handle->writerRes, &res);
	if (rc != DDS_RETCODE_OK) {
		printf("RealtimeNetSendMonitalResMsg:dds_write err[%s]。\n", dds_strretcode(-rc));
		return 0;
	}
	else
	{
		//printf("RealtimeNetMonitorSendThread:dds_write success。\n");
	}
	return 1;
}


/**
 * @brief 发送检测的节点消息
 *
 * @param inPara 输入参数，指向RtNetHandle
 * @return int 成功返回1
 *
 * 
 */
int RealtimeNetSendMonitalNodeMsg(RtNetHandle *handle)
{
	RealtimeNetMonitor_Node node;
	memset(&node, 0, sizeof(RealtimeNetMonitor_Node));
	node.processId = handle->processId;
	node.threadId = handle->threadId;
	node.ip = handle->ipAddr;
	int num = handle->topicMapListLength;
	node.topicList._length = num;
	node.topicList._maximum = num;
	TopicMapInnerItem * src = handle->topicMapList;
	RealtimeNetMonitor_Topic * des = malloc(sizeof(RealtimeNetMonitor_Topic) * num);
	
	for (int i = 0; i < num; i++) {
		des[i].size = src[i].size;
		des[i].status = src[i].status;
		des[i].name = src[i].topicName;
	}	
	node.topicList._buffer = des;
	node.topicList._release = false;

	int rc = 0;
	rc = dds_write(handle->writerNode, &node);
	if (rc != DDS_RETCODE_OK) {
		printf("RealtimeNetSendMonitalNodeMsg:dds_write err[%s]。\n", dds_strretcode(-rc));
		return 0;
	}
	else
	{
		//printf("RealtimeNetSendMonitalNodeMsg:dds_write success。\n");
	}	   
	free(des);

	return 1;

}


int IsEndWithDotOne(const char* ip) {
	size_t len = strlen(ip);
	return (len >= 2 && ip[len - 2] == '.' && ip[len - 1] == '1');
}

/**
 * @brief 获取ip地址
 *
 * @param  addr ip地址
 * @return ip地址
 *
 *
 */
void get_local_ips(char * addr) {

#ifdef _WIN32
	PIP_ADAPTER_ADDRESSES pAddresses = NULL;
	ULONG outBufLen = 0;
	GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &outBufLen);
	pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);

	if (GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen) == 0) {
		PIP_ADAPTER_ADDRESSES pCurr = pAddresses;
		while (pCurr) {
			if (pCurr->OperStatus == IfOperStatusUp &&
				!(pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
					pCurr->IfType == IF_TYPE_TUNNEL)) {
				PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurr->FirstUnicastAddress;
				while (pUnicast) {
					SOCKADDR_IN *sa_in = (SOCKADDR_IN *)pUnicast->Address.lpSockaddr;
					char * ipAddr = inet_ntoa(sa_in->sin_addr);
					if (!IsEndWithDotOne(ipAddr) && (strcmp(ipAddr, "127.0.0.1") != 0))
					{
						//printf("[WIN] Adapter: %s\tIP: %s\n",
						//	pCurr->FriendlyName, inet_ntoa(sa_in->sin_addr));
						strcpy(addr, ipAddr);
					}
					pUnicast = pUnicast->Next;
				}
			}
			pCurr = pCurr->Next;
		}
	}
	free(pAddresses);
#else
	struct ifaddrs *ifaddr, *ifa;
	if (getifaddrs(&ifaddr) == -1) return;

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;

		struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
		char *ipAddr = inet_ntoa(addr->sin_addr);
		if (!IsEndWithDotOne(ipAddr) && (strcmp(ipAddr, "127.0.0.1") != 0))
		{
			//printf("[UNIX] Interface: %s\tIP: %s\n",
			//	ifa->ifa_name, inet_ntoa(addr->sin_addr));
			strcpy(addr, ipAddr);
		}
	}
	freeifaddrs(ifaddr);

#endif

}

/**
 * @brief 获取ip地址最后.后面的数字
 *
 * @param  addr ip地址
 * @return .后面的数字
 *
 *
 */
char getIpLastChar(const char *ip) 
{
	if (!ip) return -1;

	// 查找最后一个点号的位置
	char *last_dot = strrchr(ip, '.');
	if (!last_dot || last_dot == ip + strlen(ip) - 1) {
		return -1;  // 无效格式
	}

	// 提取点号后的内容
	const char *octet_str = last_dot + 1;

	// 转换为数字
	char *endptr;
	long num = strtol(octet_str, &endptr, 10);

	// 验证转换结果
	if (*endptr != '\0' || num < 0 || num > 255) {
		return -1;  // 无效数字
	}

	return (char)num;
}
