#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dds/dds.h"
#include "dds/ddsi/ddsi_config.h"

#include "dds/ddsi/RealtimeNetMonitor_inner.h"
#include "dds/ddsi/RealtimeNetMontorData.h"
#include "dds/ddsi/RealtimeNetMsg.h"
#define MAX_SAMPLES 1	// 定义最大样本数，用于限制一次读取的最大数据样本数量
const char * 			c_topicNameReq = "q";		//发送请求的topic名
const char * 			c_topicNameRes = "s";		//接收应答的topic名
const char * 			c_topicNameNode = "n";		//接收节点的topic名

/**
 * @brief	监控的初始化
 *
 * 根据外部代码调用传来的参数，进行监控的初始化
 *
 * @param	ddsDominId		dds域id
 * @param	networkSegment	dds初始化使用的网段ip
 * @param	timeout			心跳的时间间隔，默认10秒一次
 * @param	onReceiverNode	节点信息的回调
 * @param	onReceiverHeartBeat	心跳信息的回调
 * @return
 */
void * 	MonitorInit(int ddsDominId, int timeout, OnReceiverNodeFun onReceiverNode, OnReceiverHeartBeatFun onReceiverHeartBeat, OnReceiverDataFun OnReceiverData)
{
	int ret = 0;
	RtNetMonitorHandle * handle = NULL;
	handle = malloc(sizeof(RtNetMonitorHandle));
	memset(handle, 0, sizeof(RtNetMonitorHandle));

	printf("[MonitorInit]  ddsDominId : %d\n", ddsDominId);
	printf("[MonitorInit]  timeout， : %d\n", timeout);
	handle->timeout = timeout;
	handle->onReceiverNode = onReceiverNode;
	handle->onReceiverRes = onReceiverHeartBeat;
	handle->OnReceiverData = OnReceiverData;

	if ((0 == timeout) && (NULL == onReceiverNode) && (NULL == onReceiverHeartBeat) && (NULL == OnReceiverData))
	{
		printf("参数错误！");
		return NULL;
	}


	//创建DDS参与者
	ret = RealtimeNetMonitorCreatePart(ddsDominId, handle);
	if (0 == ret) {
		return NULL;
	}
	printf("RealtimeNetMonitorCreatePart completed\n");

	//初始化handle
	ret = RealtimeNetMonitorInitHandle(handle);
	if (0 == ret) {
		return NULL;
	}

	// 创建Topic和操作器
	/* create topic and operator */
	ret = RealtimeNetMonitorCreateTopicAndOper(handle);
	if (0 == ret) {
		return NULL;
	}
	printf("RealtimeNetMonitorCreateTopicAndOper completed\n");


	// 创建条件变量
	ret = RealtimeNetMonitorCreateCond(handle);
	if (0 == ret) {
		return NULL;
	}
	printf("RealtimeNetMonitorCreateCond completed\n");
	
	// 创建消息接收发送线程
	RealtimeNetMonitorCreateThread(handle);

	return handle;
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
	const MoitorTopicItem *info = vinfo;
	// 使用偏移量和常量计算哈希值
	return (uint32_t)(((info->reader + UINT64_C(16292676669999574021)) * UINT64_C(10242350189706880077)) >> 32);
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
	const MoitorTopicItem *a = va;
	const MoitorTopicItem *b = vb;
	return a->reader == b->reader;
}

/*
 * @brief 初始化handle
 *
 * @param handle 内部全局结构体域id
 * @return int 成功返回1，失败返回0
 *
 * 该函数初始化DDS配置，创建参与者。
 */
int RealtimeNetMonitorInitHandle(RtNetMonitorHandle * handle)
{
	handle->exitFlg = false;
	handle->sendExitFlg = false;
	handle->recvExitFlg = false;
	handle->topicHead = NULL;

	// 创建哈希表，初始大小为32
	handle->readerHash = ddsrt_hh_new(32, typecachereader_hash, typecachereader_equal);

	//锁初始化
	ddsrt_mutex_init(&handle->lock);

	// 创建WaitSet
	handle->waitset = dds_create_waitset(handle->participant);

	return 1;
}

/**
 * @brief 创建DDS参与者
 *
 * @param ddsDominId dds域id
 * @param networkSegment  dds初始化使用的网段ip
 * @return int 成功返回1，失败返回0
 *
 * 该函数初始化DDS配置，创建参与者。
 */
int RealtimeNetMonitorCreatePart(int ddsDominId, RtNetMonitorHandle * handle)
{
	
	// 创建参与者
	handle->participant = dds_create_participant(ddsDominId, NULL, NULL);
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
int RealtimeNetMonitorCreateTopicAndOper(RtNetMonitorHandle *handle)
{
	//printf("===   call  RealtimeNetMonitorCreateTopicAndOper\n");
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
		printf("create topic topicReq ok.topicReq:%d\n", handle->topicReq);
	}


	//创建心跳请求的Writer
	handle->writerReq = dds_create_writer(handle->participant, handle->topicReq, NULL, NULL);
	if (handle->writerReq < 0) {
		DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-(handle->writerReq)));
		return 0;
	}
	else {
		printf("create Writer writerReq ok.Writer:%d\n", handle->writerReq);
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
		printf("create topic topicReq ok.topicReq:%d\n", handle->topicRes);
	}

	//创建心跳应答的reader
	handle->readerRes = dds_create_reader(handle->participant, handle->topicRes, NULL, NULL);
	if (handle->readerRes < 0) {
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-(handle->readerRes)));
		return 0;
	}
	else {
		printf("create reader readerRes ok.reader:%d\n", handle->readerRes);
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
		printf("create topic topicReq ok.topicReq:%d\n", handle->topicNode);
	}

	//创建心跳应答的reader
	handle->readerNode = dds_create_reader(handle->participant, handle->topicNode, NULL, NULL);
	if (handle->readerNode < 0) {
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-(handle->readerNode)));
		return 0;
	}
	else {
		printf("create reader readerRes ok.reader:%d\n", handle->readerNode);
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
int RealtimeNetMonitorCreateCond(RtNetMonitorHandle *handle)
{
	// 创建WaitSet
	handle->waitset = dds_create_waitset(handle->participant);

	dds_entity_t readcond;
	readcond  = dds_create_readcondition(handle->readerRes, DDS_ANY_STATE);
	dds_waitset_attach(handle->waitset, readcond, handle->readerRes);

	readcond = dds_create_readcondition(handle->readerNode, DDS_ANY_STATE);
	dds_waitset_attach(handle->waitset, readcond, handle->readerNode);

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
int RealtimeNetMonitorCreateThread(RtNetMonitorHandle *handle)
{
	dds_return_t rc;

	// 创建接收线程
	ddsrt_threadattr_t tattr;
	ddsrt_threadattr_init(&tattr);
	rc = ddsrt_thread_create(&(handle->recvTid), "recv", &tattr, RealtimeNetMonitorRecvThread, handle);
	   
	//等待一会儿，确认接收线程初始化完毕。
	dds_sleepfor(DDS_MSECS(100));

	// 创建发送线程
	ddsrt_threadattr_t tattr2;
	ddsrt_threadattr_init(&tattr2);
	rc = ddsrt_thread_create(&(handle->sendTid), "send", &tattr2, RealtimeNetMonitorSendThread, handle);

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
int RealtimeNetMonitorRecvThread(void * inPara)
{
	RtNetMonitorHandle *handle = inPara;
	printf("new thread ok\n");

	int ret;
	dds_attach_t triggered_reader;
	while (!handle->exitFlg) {
		//wl_add  相对时间暂时定为0.1秒
		dds_time_t tstop = dds_time() + DDS_MSECS(100);
		triggered_reader = 0;

		// 等待WaitSet触发
		//ddsrt_mutex_lock(&handle->lock);
		ret = dds_waitset_wait_until(handle->waitset, &triggered_reader, 1, tstop);
		//ddsrt_mutex_unlock(&handle->lock);
		if (ret < 0) {
			printf("dds_waitset_wait_until ret err: %d\n", ret);
			fflush(stdout);
			continue;
		}
		else if (0 == ret) {
			//printf("dds_waitset_wait_until timeout . do nothing\n");
			//fflush(stdout);
			continue;
		}
		else {
			//printf("dds_waitset_wait_until ok。 ret : %d\n", ret);
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
			continue;
		}

		// 处理有效数据
		//printf("dds_take return ok: %d\n", ret);
		if (infos[0].valid_data && (NULL != samples[0])) {
			//printf("valid_data is true\n");
			if (triggered_reader == handle->readerRes) {
				RealtimeNetMonitorDoResMsg(handle, samples[0]);
			}
			else if(triggered_reader == handle->readerNode){
				RealtimeNetMonitorDoNodeMsg(handle, samples[0]);
			}
			else {
				RealtimeNetMonitorDoDataMsg(handle, triggered_reader, samples[0]);
			}
		
			ret = dds_return_loan((dds_entity_t)triggered_reader, samples, 1);
		}
		else {
			printf("valid_data is false\n");
		}

	}

	//设置接收线程已经退出
	handle->recvExitFlg = true;
	printf("recv msg thread finish\n");
	fflush(stdout);
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
void RealtimeNetMonitorDoResMsg(RtNetMonitorHandle *handle, void * msg)
{
	RealtimeNetMonitor_Res * res = msg;
	handle->onReceiverRes(res->ip, res->processId, res->threadId);
}

/**
 * @brief 查找Topic
 *
 * @param handle 句柄
 * @param reader 哈希索引
 * @return int item
 *
 * 根据偏移量在哈希表中查找对应的Topic item。
 */
MoitorTopicItem * RealtimeNetMonitorFindTopicbyReader(RtNetMonitorHandle *handle, unsigned int reader)
{
	MoitorTopicItem inItem;
	inItem.reader = reader;
	MoitorTopicItem * pOutItem;
	//printf("RealtimeNetFindTopicbyReader: reader[%d]\n", reader);

	// 在哈希表中查找
	ddsrt_mutex_lock(&handle->lock);
	pOutItem = ddsrt_hh_lookup(handle->readerHash, &inItem);
	ddsrt_mutex_unlock(&handle->lock);

	return pOutItem;
}

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
void RealtimeNetMonitorDoDataMsg(RtNetMonitorHandle *handle, dds_attach_t triggered_reader, void * msg)
{
	int ret = 0;
	RealtimeNetData_Msg * recvMsg = msg;
	MoitorTopicItem * item = NULL;

	item = RealtimeNetMonitorFindTopicbyReader(handle, triggered_reader);
	if (NULL == item)
	{
		printf("RealtimeNetMonitorFindTopicbyReader err. triggered_reader[%d]\n", triggered_reader);
		return;
	}

	handle->OnReceiverData(item->topicName, recvMsg->data._buffer, recvMsg->data._length, recvMsg->ip, recvMsg->processId, recvMsg->threadId);
}

/**
 * @brief 处理节点应答消息
 *
 * @param inPara 输入参数，指向RtNetMonitorHandle
 * @param msg 输入参数，指向应答消息
 * @return
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
void RealtimeNetMonitorDoNodeMsg(RtNetMonitorHandle *handle, void * msg)
{
	RealtimeNetMonitor_Node *node = msg;
	RtNodeInfo nodeInfo;
	memset(&nodeInfo, 0, sizeof(RtNodeInfo));

	nodeInfo.networkSegment = node->ip;
	nodeInfo.processId = node->processId;
	nodeInfo.threadId = node->threadId;
	nodeInfo.topicMapListLength = node->topicList._length;
	if (nodeInfo.topicMapListLength != 0) {
		nodeInfo.topicMapList = malloc(sizeof(MonitorTopicItem) * nodeInfo.topicMapListLength);
	}


	for (int i = 0; i < nodeInfo.topicMapListLength; i++) {
		nodeInfo.topicMapList[i].topicName = node->topicList._buffer[i].name;
		nodeInfo.topicMapList[i].size = node->topicList._buffer[i].size;
		nodeInfo.topicMapList[i].status = node->topicList._buffer[i].status;
	}
	//printf("handle->onReceiverNode(&nodeInfo) begin\n");
	handle->onReceiverNode(&nodeInfo);
	//printf("handle->onReceiverNode(&nodeInfo) end\n");
	if (nodeInfo.topicMapListLength != 0) {
		free(nodeInfo.topicMapList);
	}
}

/**
 * @brief 消息发送线程，用于发送DDS消息
 *
 * @param inPara 输入参数，指向RtNetMonitorHandle
 * @return int 成功返回1
 *
 * 该线程从WaitSet中等待触发的Reader，读取消息并调用回调函数处理。
 */
int RealtimeNetMonitorSendThread(void * inPara)
{       
	RtNetMonitorHandle * handle = inPara;
	RealtimeNetMonitor_Req req;

	// 获取当前时间戳
	req.seq = (int)time(NULL);  
	dds_return_t rc;

	while (!handle->exitFlg)
	{
		rc = dds_write(handle->writerReq, &req);
		if (rc != DDS_RETCODE_OK) {
			printf("RealtimeNetMonitorSendThread:dds_write err[%s]。 send thread exit。\n", dds_strretcode(-rc));
			handle->sendExitFlg = true;
			return 0;
		}
		else
		{
			//printf("RealtimeNetMonitorSendThread:dds_write success。\n");
		}

		//等待配置的时长，再次发送。
		dds_sleepfor(DDS_SECS(handle->timeout));
	}


	//设置发送线程已经退出
	handle->sendExitFlg = true;
	printf("send msg thread finish\n");
	fflush(stdout);

	return 1;
}



/**
 * @brief	通信中间件的释放
 *
 *
 *
 * @param	pContext 初始化时，返回的句柄
 * @return	null
 */
void  MonitortFree(void* pContext)
{
	RtNetMonitorHandle * handle = pContext;
	handle->exitFlg = true;

	//等待发送和接收线程退出
	while (!(handle->sendExitFlg) || !(handle->recvExitFlg))
	{
		//等待配置的时长，再次发送。
		dds_sleepfor(DDS_SECS(1));
		printf("wait send and recv thread exit.\n");
	}

	// 销毁互斥锁
	ddsrt_mutex_destroy(&handle->lock);

	// 释放哈希表
	ddsrt_hh_free(handle->readerHash);

	// 删除参与者
	dds_delete(handle->participant);

	//循环释放item
	MoitorTopicItem * itemPtr = NULL;
	while (NULL != handle->topicHead)
	{
		itemPtr = handle->topicHead;
		handle->topicHead = itemPtr->nextTopic;
		free(itemPtr->topicName);
		free(itemPtr);
	}

	// 释放句柄
	free(handle);

 }


/**
 * @brief	增加topic
 *
 * 根据增加的topic,监控所有该topic的数据，通过回调返回给业务层
 *
 * @param	topicName		topic名
 * @return			函数调用结果 1-成功 0-失败
 */
int MonitorAddTopic(void* pContext, char * topicName)
{
	RtNetMonitorHandle * handle = pContext;

	//如果为空，则直接返回。
	if (NULL == topicName)
	{
		return 0;
	}

	//申请空间
	MoitorTopicItem * itemPtr = malloc(sizeof(MoitorTopicItem));
	memset(itemPtr, 0, sizeof(MoitorTopicItem));

	//赋值topic字
	int len = strlen(topicName);
	itemPtr->topicName = malloc(len + 1);
	memset(itemPtr->topicName, 0, len +1);
	strcpy(itemPtr->topicName, topicName);

	//创建topic
	itemPtr->topic = dds_create_topic(
		handle->participant, &RealtimeNetData_Msg_desc, topicName, NULL, NULL);
	if (itemPtr->topic < 0)
	{
		DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-itemPtr->topic));
		free(itemPtr->topicName);
		free(itemPtr);
		return 0;
	}

	//创建reader
	 itemPtr->reader = dds_create_reader(handle->participant, itemPtr->topic, NULL, NULL);
	if (itemPtr->reader < 0) {
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-itemPtr->reader));
		return 0;
	}

	//把item放到链表中
	itemPtr->nextTopic = handle->topicHead;
	handle->topicHead = itemPtr;

	//加锁
	ddsrt_mutex_lock(&handle->lock);

	//插入哈希
	ddsrt_hh_add(handle->readerHash, itemPtr);
	//释放锁
	ddsrt_mutex_unlock(&handle->lock);

	//添加新的topic触发条件
	dds_entity_t readcond;
	readcond = dds_create_readcondition(itemPtr->reader, DDS_ANY_STATE);
	dds_waitset_attach(handle->waitset, readcond, itemPtr->reader);



	return 1;
}