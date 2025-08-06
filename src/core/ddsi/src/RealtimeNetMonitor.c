#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dds/dds.h"
#include "dds/ddsi/ddsi_config.h"

#include "dds/ddsi/RealtimeNetMonitor_inner.h"
#include "dds/ddsi/RealtimeNetMontorData.h"
#include "dds/ddsi/RealtimeNetMsg.h"
#define MAX_SAMPLES 1	// �����������������������һ�ζ�ȡ�����������������
const char * 			c_topicNameReq = "q";		//���������topic��
const char * 			c_topicNameRes = "s";		//����Ӧ���topic��
const char * 			c_topicNameNode = "n";		//���սڵ��topic��

/**
 * @brief	��صĳ�ʼ��
 *
 * �����ⲿ������ô����Ĳ��������м�صĳ�ʼ��
 *
 * @param	ddsDominId		dds��id
 * @param	networkSegment	dds��ʼ��ʹ�õ�����ip
 * @param	timeout			������ʱ������Ĭ��10��һ��
 * @param	onReceiverNode	�ڵ���Ϣ�Ļص�
 * @param	onReceiverHeartBeat	������Ϣ�Ļص�
 * @return
 */
void * 	MonitorInit(int ddsDominId, int timeout, OnReceiverNodeFun onReceiverNode, OnReceiverHeartBeatFun onReceiverHeartBeat, OnReceiverDataFun OnReceiverData)
{
	int ret = 0;
	RtNetMonitorHandle * handle = NULL;
	handle = malloc(sizeof(RtNetMonitorHandle));
	memset(handle, 0, sizeof(RtNetMonitorHandle));

	printf("[MonitorInit]  ddsDominId : %d\n", ddsDominId);
	printf("[MonitorInit]  timeout�� : %d\n", timeout);
	handle->timeout = timeout;
	handle->onReceiverNode = onReceiverNode;
	handle->onReceiverRes = onReceiverHeartBeat;
	handle->OnReceiverData = OnReceiverData;

	if ((0 == timeout) && (NULL == onReceiverNode) && (NULL == onReceiverHeartBeat) && (NULL == OnReceiverData))
	{
		printf("��������");
		return NULL;
	}


	//����DDS������
	ret = RealtimeNetMonitorCreatePart(ddsDominId, handle);
	if (0 == ret) {
		return NULL;
	}
	printf("RealtimeNetMonitorCreatePart completed\n");

	//��ʼ��handle
	ret = RealtimeNetMonitorInitHandle(handle);
	if (0 == ret) {
		return NULL;
	}

	// ����Topic�Ͳ�����
	/* create topic and operator */
	ret = RealtimeNetMonitorCreateTopicAndOper(handle);
	if (0 == ret) {
		return NULL;
	}
	printf("RealtimeNetMonitorCreateTopicAndOper completed\n");


	// ������������
	ret = RealtimeNetMonitorCreateCond(handle);
	if (0 == ret) {
		return NULL;
	}
	printf("RealtimeNetMonitorCreateCond completed\n");
	
	// ������Ϣ���շ����߳�
	RealtimeNetMonitorCreateThread(handle);

	return handle;
}


/**
 * @brief �����ϣֵ������ͨ��reader�洢�Ͳ���TopicMapInnerItem
 *
 * @param vinfo ָ��TopicMapInnerItem��ָ��
 * @return uint32_t ������Ĺ�ϣֵ
 *
 * �ú���ͨ��ƫ�������������������ϣֵ��ȷ����ϣ��ķֲ����ȡ�
 */
static uint32_t typecachereader_hash(const void *vinfo)
{
	const MoitorTopicItem *info = vinfo;
	// ʹ��ƫ�����ͳ��������ϣֵ
	return (uint32_t)(((info->reader + UINT64_C(16292676669999574021)) * UINT64_C(10242350189706880077)) >> 32);
}


/**
 * @brief ͨ��reader�Ƚ�����TopicMapInnerItem�Ƿ����
 *
 * @param va ָ���һ��TopicMapInnerItem��ָ��
 * @param vb ָ��ڶ���TopicMapInnerItem��ָ��
 * @return int �����ȷ���1�����򷵻�0
 *
 * �ú���ͨ���Ƚ�����TopicMapInnerItem��`offset`�ֶ����ж������Ƿ���ȡ�
 */
static int typecachereader_equal(const void *va, const void *vb)
{
	const MoitorTopicItem *a = va;
	const MoitorTopicItem *b = vb;
	return a->reader == b->reader;
}

/*
 * @brief ��ʼ��handle
 *
 * @param handle �ڲ�ȫ�ֽṹ����id
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú�����ʼ��DDS���ã����������ߡ�
 */
int RealtimeNetMonitorInitHandle(RtNetMonitorHandle * handle)
{
	handle->exitFlg = false;
	handle->sendExitFlg = false;
	handle->recvExitFlg = false;
	handle->topicHead = NULL;

	// ������ϣ����ʼ��СΪ32
	handle->readerHash = ddsrt_hh_new(32, typecachereader_hash, typecachereader_equal);

	//����ʼ��
	ddsrt_mutex_init(&handle->lock);

	// ����WaitSet
	handle->waitset = dds_create_waitset(handle->participant);

	return 1;
}

/**
 * @brief ����DDS������
 *
 * @param ddsDominId dds��id
 * @param networkSegment  dds��ʼ��ʹ�õ�����ip
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú�����ʼ��DDS���ã����������ߡ�
 */
int RealtimeNetMonitorCreatePart(int ddsDominId, RtNetMonitorHandle * handle)
{
	
	// ����������
	handle->participant = dds_create_participant(ddsDominId, NULL, NULL);
	if (handle->participant < 0) {
		DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-handle->participant));
		return 0;
	}
	
	return 1;
}

/**
 * @brief ����Topic�Ͳ�������Reader/Writer��
 *
 * @param handle ���������Topicӳ���б�
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú���Ϊÿ��Topic������Ӧ��Reader��Writer��������洢������С�
 */
int RealtimeNetMonitorCreateTopicAndOper(RtNetMonitorHandle *handle)
{
	//printf("===   call  RealtimeNetMonitorCreateTopicAndOper\n");
	int ret = 0;

	//�������������topic
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


	//�������������Writer
	handle->writerReq = dds_create_writer(handle->participant, handle->topicReq, NULL, NULL);
	if (handle->writerReq < 0) {
		DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-(handle->writerReq)));
		return 0;
	}
	else {
		printf("create Writer writerReq ok.Writer:%d\n", handle->writerReq);
	}

	//��������Ӧ���topic
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

	//��������Ӧ���reader
	handle->readerRes = dds_create_reader(handle->participant, handle->topicRes, NULL, NULL);
	if (handle->readerRes < 0) {
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-(handle->readerRes)));
		return 0;
	}
	else {
		printf("create reader readerRes ok.reader:%d\n", handle->readerRes);
	}


	//�����ڵ�Ӧ���topic
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

	//��������Ӧ���reader
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
 * @brief ��������������ע�ᵽWaitSet
 *
 * @param handle ���������Reader�б�
 * @return int �ɹ�����1
 *
 * �ú���Ϊÿ��Reader����ReadCondition��������ע�ᵽWaitSet�С�
 */
int RealtimeNetMonitorCreateCond(RtNetMonitorHandle *handle)
{
	// ����WaitSet
	handle->waitset = dds_create_waitset(handle->participant);

	dds_entity_t readcond;
	readcond  = dds_create_readcondition(handle->readerRes, DDS_ANY_STATE);
	dds_waitset_attach(handle->waitset, readcond, handle->readerRes);

	readcond = dds_create_readcondition(handle->readerNode, DDS_ANY_STATE);
	dds_waitset_attach(handle->waitset, readcond, handle->readerNode);

	return 1;
}

/**
 * @brief ������Ϣ�����߳�
 *
 * @param handle ���������Reader�б�
 * @return int �ɹ�����1
 *
 * �������Reader���򴴽�һ���߳����ڽ�����Ϣ��
 */
int RealtimeNetMonitorCreateThread(RtNetMonitorHandle *handle)
{
	dds_return_t rc;

	// ���������߳�
	ddsrt_threadattr_t tattr;
	ddsrt_threadattr_init(&tattr);
	rc = ddsrt_thread_create(&(handle->recvTid), "recv", &tattr, RealtimeNetMonitorRecvThread, handle);
	   
	//�ȴ�һ�����ȷ�Ͻ����̳߳�ʼ����ϡ�
	dds_sleepfor(DDS_MSECS(100));

	// ���������߳�
	ddsrt_threadattr_t tattr2;
	ddsrt_threadattr_init(&tattr2);
	rc = ddsrt_thread_create(&(handle->sendTid), "send", &tattr2, RealtimeNetMonitorSendThread, handle);

	return 1;
}

/**
 * @brief ��Ϣ�����̣߳����ڴ�����յ���DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
int RealtimeNetMonitorRecvThread(void * inPara)
{
	RtNetMonitorHandle *handle = inPara;
	printf("new thread ok\n");

	int ret;
	dds_attach_t triggered_reader;
	while (!handle->exitFlg) {
		//wl_add  ���ʱ����ʱ��Ϊ0.1��
		dds_time_t tstop = dds_time() + DDS_MSECS(100);
		triggered_reader = 0;

		// �ȴ�WaitSet����
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
			//printf("dds_waitset_wait_until ok�� ret : %d\n", ret);
			//fflush(stdout);
		}

		// ��ȡ��Ϣ
		void *samples[MAX_SAMPLES] = { NULL };
		dds_sample_info_t infos[MAX_SAMPLES];
		ret = dds_take((dds_entity_t)triggered_reader, samples, infos, 1, 1);
		if (ret <= 0)
		{
			printf("dds_take ret <=0: %d�� continue run\n", ret);
			fflush(stdout);
			continue;
		}

		// ������Ч����
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

	//���ý����߳��Ѿ��˳�
	handle->recvExitFlg = true;
	printf("recv msg thread finish\n");
	fflush(stdout);
	return 1;
}

/**
 * @brief ��Ϣ�����̣߳����ڴ�����յ���DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
void RealtimeNetMonitorDoResMsg(RtNetMonitorHandle *handle, void * msg)
{
	RealtimeNetMonitor_Res * res = msg;
	handle->onReceiverRes(res->ip, res->processId, res->threadId);
}

/**
 * @brief ����Topic
 *
 * @param handle ���
 * @param reader ��ϣ����
 * @return int item
 *
 * ����ƫ�����ڹ�ϣ���в��Ҷ�Ӧ��Topic item��
 */
MoitorTopicItem * RealtimeNetMonitorFindTopicbyReader(RtNetMonitorHandle *handle, unsigned int reader)
{
	MoitorTopicItem inItem;
	inItem.reader = reader;
	MoitorTopicItem * pOutItem;
	//printf("RealtimeNetFindTopicbyReader: reader[%d]\n", reader);

	// �ڹ�ϣ���в���
	ddsrt_mutex_lock(&handle->lock);
	pOutItem = ddsrt_hh_lookup(handle->readerHash, &inItem);
	ddsrt_mutex_unlock(&handle->lock);

	return pOutItem;
}

/**
 * @brief ����ҵ��������Ϣ
 *
 * @param inPara ���������ָ��RtNetMonitorHandle
 * @param triggered_reader ��Ϣ��Ӧ��reader
 * @param msg ���������ָ��Ӧ����Ϣ
 * @return
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
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
 * @brief ����ڵ�Ӧ����Ϣ
 *
 * @param inPara ���������ָ��RtNetMonitorHandle
 * @param msg ���������ָ��Ӧ����Ϣ
 * @return
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
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
 * @brief ��Ϣ�����̣߳����ڷ���DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetMonitorHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
int RealtimeNetMonitorSendThread(void * inPara)
{       
	RtNetMonitorHandle * handle = inPara;
	RealtimeNetMonitor_Req req;

	// ��ȡ��ǰʱ���
	req.seq = (int)time(NULL);  
	dds_return_t rc;

	while (!handle->exitFlg)
	{
		rc = dds_write(handle->writerReq, &req);
		if (rc != DDS_RETCODE_OK) {
			printf("RealtimeNetMonitorSendThread:dds_write err[%s]�� send thread exit��\n", dds_strretcode(-rc));
			handle->sendExitFlg = true;
			return 0;
		}
		else
		{
			//printf("RealtimeNetMonitorSendThread:dds_write success��\n");
		}

		//�ȴ����õ�ʱ�����ٴη��͡�
		dds_sleepfor(DDS_SECS(handle->timeout));
	}


	//���÷����߳��Ѿ��˳�
	handle->sendExitFlg = true;
	printf("send msg thread finish\n");
	fflush(stdout);

	return 1;
}



/**
 * @brief	ͨ���м�����ͷ�
 *
 *
 *
 * @param	pContext ��ʼ��ʱ�����صľ��
 * @return	null
 */
void  MonitortFree(void* pContext)
{
	RtNetMonitorHandle * handle = pContext;
	handle->exitFlg = true;

	//�ȴ����ͺͽ����߳��˳�
	while (!(handle->sendExitFlg) || !(handle->recvExitFlg))
	{
		//�ȴ����õ�ʱ�����ٴη��͡�
		dds_sleepfor(DDS_SECS(1));
		printf("wait send and recv thread exit.\n");
	}

	// ���ٻ�����
	ddsrt_mutex_destroy(&handle->lock);

	// �ͷŹ�ϣ��
	ddsrt_hh_free(handle->readerHash);

	// ɾ��������
	dds_delete(handle->participant);

	//ѭ���ͷ�item
	MoitorTopicItem * itemPtr = NULL;
	while (NULL != handle->topicHead)
	{
		itemPtr = handle->topicHead;
		handle->topicHead = itemPtr->nextTopic;
		free(itemPtr->topicName);
		free(itemPtr);
	}

	// �ͷž��
	free(handle);

 }


/**
 * @brief	����topic
 *
 * �������ӵ�topic,������и�topic�����ݣ�ͨ���ص����ظ�ҵ���
 *
 * @param	topicName		topic��
 * @return			�������ý�� 1-�ɹ� 0-ʧ��
 */
int MonitorAddTopic(void* pContext, char * topicName)
{
	RtNetMonitorHandle * handle = pContext;

	//���Ϊ�գ���ֱ�ӷ��ء�
	if (NULL == topicName)
	{
		return 0;
	}

	//����ռ�
	MoitorTopicItem * itemPtr = malloc(sizeof(MoitorTopicItem));
	memset(itemPtr, 0, sizeof(MoitorTopicItem));

	//��ֵtopic��
	int len = strlen(topicName);
	itemPtr->topicName = malloc(len + 1);
	memset(itemPtr->topicName, 0, len +1);
	strcpy(itemPtr->topicName, topicName);

	//����topic
	itemPtr->topic = dds_create_topic(
		handle->participant, &RealtimeNetData_Msg_desc, topicName, NULL, NULL);
	if (itemPtr->topic < 0)
	{
		DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-itemPtr->topic));
		free(itemPtr->topicName);
		free(itemPtr);
		return 0;
	}

	//����reader
	 itemPtr->reader = dds_create_reader(handle->participant, itemPtr->topic, NULL, NULL);
	if (itemPtr->reader < 0) {
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-itemPtr->reader));
		return 0;
	}

	//��item�ŵ�������
	itemPtr->nextTopic = handle->topicHead;
	handle->topicHead = itemPtr;

	//����
	ddsrt_mutex_lock(&handle->lock);

	//�����ϣ
	ddsrt_hh_add(handle->readerHash, itemPtr);
	//�ͷ���
	ddsrt_mutex_unlock(&handle->lock);

	//����µ�topic��������
	dds_entity_t readcond;
	readcond = dds_create_readcondition(itemPtr->reader, DDS_ANY_STATE);
	dds_waitset_attach(handle->waitset, readcond, itemPtr->reader);



	return 1;
}