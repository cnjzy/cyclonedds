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

#define MAX_SAMPLES 1	// �����������������������һ�ζ�ȡ�����������������

/**
 * @brief �����ϣֵ������ͨ��offset�洢�Ͳ���TopicMapInnerItem
 *
 * @param vinfo ָ��TopicMapInnerItem��ָ��
 * @return uint32_t ������Ĺ�ϣֵ
 *
 * �ú���ͨ��ƫ�������������������ϣֵ��ȷ����ϣ��ķֲ����ȡ�
 */
static uint32_t typecache_hash (const void *vinfo)
{
	const TopicMapInnerItem *info = vinfo;
	// ʹ��ƫ�����ͳ��������ϣֵ
	return (uint32_t) (((info->offset + UINT64_C (16292676669999574021)) * UINT64_C (10242350189706880077)) >> 32);   
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
	const TopicMapInnerItem *info = vinfo;
	// ʹ��ƫ�����ͳ��������ϣֵ
	return (uint32_t)(((info->operator + UINT64_C(16292676669999574021)) * UINT64_C(10242350189706880077)) >> 32);
}

/**
 * @brief ͨ��offset�Ƚ�����TopicMapInnerItem�Ƿ����
 *
 * @param va ָ���һ��TopicMapInnerItem��ָ��
 * @param vb ָ��ڶ���TopicMapInnerItem��ָ��
 * @return int �����ȷ���1�����򷵻�0
 *
 * �ú���ͨ���Ƚ�����TopicMapInnerItem��`offset`�ֶ����ж������Ƿ���ȡ�
 */
static int typecache_equal (const void *va, const void *vb)
{
  const TopicMapInnerItem *a = va;
  const TopicMapInnerItem *b = vb;
  return a->offset == b->offset;
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
	const TopicMapInnerItem *a = va;
	const TopicMapInnerItem *b = vb;
	return a->operator == b->operator;
}

/**
 * @brief ��ʼ��������������õ������
 *
 * @param initParam ��ʼ����������������Ρ�Topicӳ���б����Ϣ
 * @param handle ��������ڴ洢��ʼ����Ĳ���
 *
 * �ú�����`initParam`�еĲ������Ƶ�`handle`�У��������Ҫ���ڴ档
 * ���δ���ó�ʱʱ�䣬��Ĭ������Ϊ1Сʱ��
 */
void RealtimeNetSetParam(RtNetInitParam *initParam, RtNetHandle * handle)
{
	// ����DDS��ID
	handle->ddsDominId = initParam->ddsDominId;


	// �����������Ϣ
	size_t length = 0;
	if (NULL != initParam->networkSegment)
	{
		length = strlen(initParam->networkSegment);
	}
	handle->networkSegment = malloc(length + 1);
	memcpy(handle->networkSegment, initParam->networkSegment, length);
	handle->networkSegment[length] = '\0';

	// ��ʼ��Topicӳ���б�
	handle->topicMapListLength = initParam->topicMapListLength;
	TopicMapInnerItem *desList = malloc(sizeof(TopicMapInnerItem) * handle->topicMapListLength);
	memset(desList, 0, sizeof(TopicMapInnerItem) * handle->topicMapListLength);
	TopicMapItem *srcList = initParam->topicMapList;
	for (int i = 0; i < handle->topicMapListLength; i++) {
		OS_LOG_INFO(handle->log->pad_log_handle_fp, "Mapping: Offset=%u, TopicName=%s", srcList[i].offset, srcList[i].topicName);
		// ����Topic����
		length = strlen(srcList[i].topicName);
		desList[i].topicName = malloc(length + 1);
		memset(desList[i].topicName, 0, length + 1);
		memcpy(desList[i].topicName, srcList[i].topicName, length);
		//strncpy(desList[i].topicName, srcList[i].topicName, length);

		// ���������ֶ�
		desList[i].offset = srcList[i].offset;
		desList[i].size = srcList[i].size;
		desList[i].status = srcList[i].status;
		desList[i].buf = NULL;

		// ����Ƕ�ȡ���͵�Topic�����仺����
		if (TOPIC_TYPE_READ == desList[i].status) {
			desList[i].buf = malloc(desList[i].size + 1);
			memset(desList[i].buf, 0, desList[i].size + 1);
		}
		desList[i].num = i;		// ����Topic������
	}
	handle->topicMapList = desList;

	// ���ûص������ͳ�ʱʱ��
	handle->onReceiverTopic = initParam->onReceiverTopic;

	//���ü����ص���Ϣ
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

	// ��ʼ���˳���־�ͻ�����
	handle->exitFlg = false;
	ddsrt_mutex_init(&handle->lock);
}

/**
 * @brief ��ʼ����ϣ�����ڴ洢TopicMapInnerItem
 *
 * @param handle ���������Topicӳ���б�
 * @return int �ɹ�����1
 *
 * �ú�������һ����ϣ������Topicӳ���б��е�ÿ��Ԫ����ӵ���ϣ���С�
 */
int RealtimeNetInitHash(RtNetHandle *handle)
{
	// ����offset read��ϣ����ʼ��СΪ32
	handle->offsetReadHash = ddsrt_hh_new (32, typecache_hash, typecache_equal);
	// ����offset write��ϣ����ʼ��СΪ32
	handle->offsetWriteHash = ddsrt_hh_new(32, typecache_hash, typecache_equal);

	// ��Topicӳ���б��е�ÿ��Ԫ����ӵ���ϣ����
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

	// ����reader��ϣ����ʼ��СΪ32
	handle->readerHash = ddsrt_hh_new(32, typecachereader_hash, typecachereader_equal);

	// ��Topicӳ���б��е�ÿ��Ԫ����ӵ���ϣ����
	for (int i = 0; i < handle->topicMapListLength; i++) {
		ddsrt_hh_add(handle->readerHash, &handle->topicMapList[i]);
		//printf("ddsrt_hh_add: operator[%d]\n", handle->topicMapList[i].operator);
	}

	return 1;
}

/**
 * @brief ����DDS������
 *
 * @param handle ����������������Ϣ
 * @param myQos  Qos
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú�����ʼ��DDS���ã����������ߣ�������洢������С�
 */
int RealtimeNetCreatePart(RtNetHandle *handle, Qos * myQos)
{
	// ��ʼ��Ĭ������
	ddsi_config_init_default(&handle->raw);
	handle->raw.depr_networkAddressString = handle->networkSegment;
	
	// ����DDS��
	dds_create_domain_with_rawconfig(handle->ddsDominId, &handle->raw);

	//����qos
	handle->qos = dds_create_qos();
	//����qos
	RealtimeNetReadConf_setQos(handle->qos, myQos);

	// ����������
	handle->participant = dds_create_participant(handle->ddsDominId, handle->qos, NULL);
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
int RealtimeNetCreateTopicAndOper(RtNetHandle *handle)
{
	int ret = 0;
	// ����ҵ����ص�Topic�Ͳ�����
	ret = RealtimeNetCreateNormalTopicAndOper(handle);
	if (0 == ret)
	{
		OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetCreateNormalTopicAndOper: call RealtimeNetCreateNormalTopicAndOper err. ");
		return ret;
	}

	// ���������ص�Topic�Ͳ�����
	ret = RealtimeNetCreatMonitorTopicAndOper(handle);
	if (0 == ret)
	{
		OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetCreateNormalTopicAndOper: call RealtimeNetCreatMonitorTopicAndOper err. ");
		return ret;
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
int RealtimeNetCreateNormalTopicAndOper(RtNetHandle *handle)
{
	//printf("===   call  RealtimeNetCreateNormalTopicAndOper\n");
	int ret = 0;

	// ����QoS���ã�����Ϊ���Ŭ���ɿ���
	/* Create a reliable Reader. */
	//dds_qos_t *qos;
	//qos = dds_create_qos();
	//dds_qset_reliability(qos, DDS_RELIABILITY_BEST_EFFORT, 0);

	// ����Topicӳ���б�Ϊÿ��Topic����Reader��Writer
	for (int i = 0; i < handle->topicMapListLength; i++) {
		/* Create a Topic. */
		// ����Topic
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

		// ����Topic���ʹ���Writer��Reader
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
	// ɾ��QoS����
	//dds_delete_qos(qos);
	return ret;
}

/**
 * @brief ���������ص�Topic�Ͳ�������Reader/Writer��
 *
 * @param handle ���������Topicӳ���б�
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú���Ϊÿ��Topic������Ӧ��Reader��Writer��������洢������С�
 */
int RealtimeNetCreatMonitorTopicAndOper(RtNetHandle *handle)
{
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
		//printf("create topic topicReq ok.topicReq:%d\n", handle->topicReq);
	}


	//�������������reader
	handle->readerReq = dds_create_reader(handle->participant, handle->topicReq, NULL, NULL);
	if (handle->readerReq < 0) {
		DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-(handle->readerReq)));
		return 0;
	}
	else {
		//printf("create Writer writerReq ok.Writer:%d\n", handle->readerReq);
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
		//printf("create topic topicReq ok.topicReq:%d\n", handle->topicRes);
	}

	//��������Ӧ���writer
	handle->writerRes = dds_create_writer(handle->participant, handle->topicRes, NULL, NULL);
	if (handle->writerRes < 0) {
		DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-(handle->writerRes)));
		return 0;
	}
	else {
		//printf("create reader readerRes ok.reader:%d\n", handle->writerRes);
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
		//printf("create topic topicReq ok.topicReq:%d\n", handle->topicNode);
	}

	//�����ڵ�Ӧ���reader
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
 * @brief ��������������ע�ᵽWaitSet
 *
 * @param handle ���������Reader�б�
 * @return int �ɹ�����1
 *
 * �ú���Ϊÿ��Reader����ReadCondition��������ע�ᵽWaitSet�С�
 */
int RealtimeNetCreateCond(RtNetHandle *handle)
{
	//dds_entity_t waitset;
	dds_entity_t reader;
	dds_entity_t readcond;

	// ����WaitSet
	handle->waitset = dds_create_waitset(handle->participant);
	//����readerע����������
	for (int i = 0; i < handle->topicMapListLength; i++) {
		if(TOPIC_TYPE_READ == handle->topicMapList[i].status) {
			reader = handle->topicMapList[i].operator;
			readcond = dds_create_readcondition(reader, DDS_ANY_STATE);
			dds_waitset_attach(handle->waitset, readcond, reader);
		}
	}

	//��ص�readerע����������
	readcond = dds_create_readcondition(handle->readerReq, DDS_ANY_STATE);
	dds_waitset_attach(handle->waitset, readcond, handle->readerReq);

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
int RealtimeNetMsgThread(void * inPara)
{
	RtNetHandle *handle = inPara;
	//printf("new thread ok\n");
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "new thread ok");
	dds_sleepfor(DDS_SECS(3));


	int ret;
	dds_attach_t triggered_reader;
	while (!handle->exitFlg) {
		//wl_add  ���ʱ����ʱ��λ1��
		dds_time_t tstop = dds_time() + DDS_SECS(1);
		triggered_reader = 0;

		// �ȴ�WaitSet����
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
			//printf("dds_waitset_wait_until ok�� ret : %d\n", ret);
			//OS_LOG_INFO(handle->log->pad_log_handle_fp, "dds_waitset_wait_until ok�� ret : %d", ret);
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
			//dds_sleepfor(DDS_SECS(10));
			continue;
		}

		// ������Ч����
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
 * @brief ������Ϣ�����߳�
 *
 * @param handle ���������Reader�б�
 * @return int �ɹ�����1
 *
 * �������Reader���򴴽�һ���߳����ڽ�����Ϣ��
 */
int RealtimeNetCreateThread(RtNetHandle *handle)
{
	//֧�ּ�ع��ܺ�����dds�����Ҫ���������̡߳�
	/*
	bool haveReader = false;

	//����readerע����������
	for (int i = 0; i < handle->topicMapListLength; i++) {
		if (TOPIC_TYPE_READ == handle->topicMapList[i].status) {
			haveReader = true;
			break;
		}
	}

	// ���û��Reader��ֱ�ӷ���
	if (!haveReader)
		return 1;
	*/

	// �����߳�
	dds_return_t rc;
	ddsrt_threadattr_t tattr;
	ddsrt_threadattr_init(&tattr);
	rc = ddsrt_thread_create(&(handle->tid), "recv", &tattr, RealtimeNetMsgThread, handle);	
	return 1;
}

/**
 * @brief ��ʼ��ʵʱ����
 *
 * @param initParam ��ʼ������
 * @return void* �ɹ����ؾ����ʧ�ܷ���NULL
 *
 * �ú������ε��ø�����ʼ�����������ʵʱ����ĳ�ʼ����
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

	// ��ʼ����־
	LogInit(handle, handle->pConf->log.logLevel);
	//�ȴ���־ϵͳ�������
	dds_sleepfor(DDS_MSECS(30));

	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetInit: Start initialization");
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "Parameters: ddsDominId=%d, networkSegment=%s, topicMapListLength=%d",
		initParam.ddsDominId, initParam.networkSegment, initParam.topicMapListLength);

	int ret = 0;

/*  //��Ȩ������ʱע�͵�����������
	//��Ȩ
	ret = RealtimeNetAuthentication(pConf->lice.licencePath);
	if (!ret) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetAuthentication failed");
		return NULL;
	}
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetAuthentication completed");
*/


	// ���ò���
	/*inputpara to handle */
	RealtimeNetSetParam(&initParam, handle);
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetSetParam completed");


	// ����������
	  /* Create a Participant. */
	ret = RealtimeNetCreatePart(handle, &(handle->pConf->qos));
	if (0 == ret) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetCreatePart failed");
		return NULL;
	}
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetCreatePart completed");
	
	// ����Topic�Ͳ�����
	/* create topic and operator */
	ret = RealtimeNetCreateTopicAndOper(handle);
	if (0 == ret)	{
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetCreateTopicAndOper failed");
		return NULL;
	}
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetCreateTopicAndOper completed");
	   

	// ��ʼ����ϣ��
	RealtimeNetInitHash(handle);
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetInitHash completed");

	// ������������
	/* create cond */
	ret = RealtimeNetCreateCond(handle);
	if (0 == ret) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetCreateCond failed");
		return NULL;
	}
	OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetCreateCond completed");

	// ������Ϣ�����߳�
	//create thread read msg
	RealtimeNetCreateThread(handle);
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetInit: Initialization completed successfully");

	long long end = os_getCurrentEpochTime();
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetInit Operation took %f seconds", (double)(end - start) / 1000);


	//RealtimeNetReadConf_free(pConf);

	//��ֹ����������ò��õĻ�����ʼ��δ��ɣ���ʱһ�����
	dds_sleepfor(DDS_MSECS(100));

	//����һ�������Ϣ
	RealtimeNetSendMonitalNodeMsg(handle);
	return handle;
}

/**
 * @brief ��ȡ����
 *
 * @param pContext �����ľ��
 * @param offset ƫ����
 * @param pBuffer ������
 * @param size ���ݴ�С
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * ����ƫ�������Ҷ�Ӧ��Topic���������ݸ��Ƶ��������С�
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

	// ����Topic
	int num = RealtimeNetFindTopicbyReadOffset(handle, offset);
	if (num < 0) {
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetRead: Invalid offset=%u. Topic not found.", offset);
		printf("offset[%d] not found topic. read error.\n", offset);
		return 0;
	}

	// ������ݴ�С�Ƿ�ƥ��
	if (size != handle->topicMapList[num].size) {
		printf("size is error. inputsize[%d], topicsize[%d]\n", size, handle->topicMapList[num].size);
		OS_LOG_DEBUG(handle->log->pad_log_handle_fp, "RealtimeNetRead: Size mismatch. InputSize=%u, TopicSize=%u", size, handle->topicMapList[num].size);
		return 0;
	}

	//��������ݣ����ṩ
	if (handle->topicMapList[num].haveDataFlg)
	{
		ret = 1;
		// �������ݵ�������
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
 * @brief ����Topic
 *
 * @param handle ���
 * @param offset ƫ����
 * @return int �ɹ�����Topic������ʧ�ܷ���-1
 *
 * ����ƫ�����ڹ�ϣ���в��Ҷ�Ӧ��Topic��
 */
int RealtimeNetFindTopicbyReadOffset(RtNetHandle *handle, unsigned int offset)
{
	TopicMapInnerItem inItem;
	inItem.offset = offset;
	TopicMapInnerItem * pOutItem;

	// �ڹ�ϣ���в���
	pOutItem = ddsrt_hh_lookup (handle->offsetReadHash, &inItem);
	if (NULL == pOutItem) {
		return -1;
	}
	//printf("lookup result. offset[%d], topicname[%s], num[%d]\n", offset, pOutItem->topicName, pOutItem->num);

	return pOutItem->num;

}

/**
 * @brief ����Topic
 *
 * @param handle ���
 * @param offset ƫ����
 * @return int �ɹ�����Topic������ʧ�ܷ���-1
 *
 * ����ƫ�����ڹ�ϣ���в��Ҷ�Ӧ��Topic��
 */
int RealtimeNetFindTopicbyWriteOffset(RtNetHandle *handle, unsigned int offset)
{
	TopicMapInnerItem inItem;
	inItem.offset = offset;
	TopicMapInnerItem * pOutItem;

	// �ڹ�ϣ���в���
	pOutItem = ddsrt_hh_lookup(handle->offsetWriteHash, &inItem);
	if (NULL == pOutItem) {
		return -1;
	}
	//printf("lookup result. offset[%d], topicname[%s], num[%d]\n", offset, pOutItem->topicName, pOutItem->num);

	return pOutItem->num;

}

/**
 * @brief ����Topic
 *
 * @param handle ���
 * @param reader ��ϣ����
 * @return int �ɹ�����Topic������ʧ�ܷ���-1
 *
 * ����ƫ�����ڹ�ϣ���в��Ҷ�Ӧ��Topic��
 */
int RealtimeNetFindTopicbyReader(RtNetHandle *handle, unsigned int reader)
{
	TopicMapInnerItem inItem;
	inItem.operator = reader;
	TopicMapInnerItem * pOutItem;
	//printf("RealtimeNetFindTopicbyReader: reader[%d]\n", reader);

	// �ڹ�ϣ���в���
	pOutItem = ddsrt_hh_lookup(handle->readerHash, &inItem);
	if (NULL == pOutItem) {
		return -1;
	}
	//printf("lookup result. reader[%d], topicname[%s], num[%d]\n", reader, pOutItem->topicName, pOutItem->num);

	return pOutItem->num;

}

/**
 * @brief ��ʼ����־
 *
 * @param handle ���
 * @return int �ɹ�����0
 *
 * �ú������ڳ�ʼ����־ϵͳ����ǰʵ��Ϊ�ա�
 */
int LogInit(RtNetHandle *handle, os_log_level level)
{

	/* ��ʼ����־ģ�� */
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
 * @brief д������
 *
 * @param pContext �����ľ��
 * @param offset ƫ����
 * @param pBuffer ���ݻ�����
 * @param size ���ݴ�С
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * ����ƫ�������Ҷ�Ӧ��Topic����������д�롣
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

	// ����Topic
	int num = RealtimeNetFindTopicbyWriteOffset(handle, offset);
	if (num < 0) {
		printf("offset[%d] not found topic. write error.\n", offset);
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetWrite: Invalid offset=%u. Topic not found.", offset);
		return 0;
	}

	// ������ݴ�С�Ƿ�ƥ��
	if (size != handle->topicMapList[num].size) {
		printf("input size is error. input size[%d], topic size[%d]\n", size, handle->topicMapList[num].size);
		OS_LOG_ERROR(handle->log->pad_log_handle_fp, "RealtimeNetWrite: Size mismatch. InputSize=%u, TopicSize=%u", size, handle->topicMapList[num].size);
		return 0;

	}

	// ������Ϣ��д��
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
 * @brief �ͷ���Դ
 *
 * @param pContext �����ľ��
 *
 * �ͷž���з����������Դ�������ڴ桢�̡߳��������ȡ�
 */
void  RealtimeNetFree(void* pContext)
{
	long long start = os_getCurrentEpochTime();
	RtNetHandle *handle;
	handle = pContext;

	// �����˳���־���ȴ��߳��˳�
	handle->exitFlg = true;
	while (!handle->recvThreadExitFlg)
	{
		dds_sleepfor(DDS_MSECS(100));
	}
	printf("�����߳��˳���������Ҳ�˳���\n");
	fflush(stdout);

	// �ͷ�QOS����
	RealtimeNetReadConf_free(handle->pConf);
	//�ͷ�qos
	dds_delete_qos(handle->qos);

	// ���ٻ�����
	ddsrt_mutex_destroy(&handle->lock);
	printf("===  call  RealtimeNetFree\n");

	// �ͷŹ�ϣ��
	ddsrt_hh_free(handle->offsetReadHash);
	ddsrt_hh_free(handle->offsetWriteHash);
	ddsrt_hh_free(handle->readerHash);

	// ɾ��������
	dds_delete(handle->participant);

	// �ͷ�Topicӳ���б��е���Դ
	TopicMapInnerItem *desList = handle->topicMapList;
	for (int i = 0; i < handle->topicMapListLength; i++) {
		free(desList[i].topicName);
		if (TOPIC_TYPE_READ == desList[i].status) {
			free(desList[i].buf);
		}
	}	
	free(handle->topicMapList);	

	// �ͷ��������Ϣ�;��
	free(handle->networkSegment);


	long long end = os_getCurrentEpochTime();
	OS_LOG_INFO(handle->log->pad_log_handle_fp, "RealtimeNetFree Operation took %f seconds", (double)(end - start) / 1000);
	
	// �ͷ���־�����������Դ
	free(handle->log);
	free(handle);
}
/**
 * @brief ������־����
 *
 * @param handle ���
 * @param level ��־����
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú���������־ģ���Ĭ�ϼ���
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
 * @brief	������־�ļ�
 *
 * @param handle ���
 * @param	filePath	��־����
 * @param	fileName	��־����
 * @param	fileMaxSize	��־�ļ�����С���ֽڣ�
 * @return	�������ý�� 1-�ɹ� 0-ʧ��
 */
int setLogFile(RtNetHandle *handle, char filePath[256], char fileName[256], int fileMaxSize)
{
	if (!handle->log)
	{
		return 0;
	}
	// ������־·��
	strncpy(handle->log->pad_log_handle_fp->path, filePath, sizeof(handle->log->pad_log_handle_fp->path) - 1);
	handle->log->pad_log_handle_fp->path[sizeof(handle->log->pad_log_handle_fp->path) - 1] = '\0'; // Ensure null-termination 
	// ������־�ļ���
	strncpy(handle->log->pad_log_handle_fp->log_file_name, fileName, sizeof(handle->log->pad_log_handle_fp->log_file_name) - 1);
	handle->log->pad_log_handle_fp->log_file_name[sizeof(handle->log->pad_log_handle_fp->log_file_name) - 1] = '\0'; // Ensure null-termination 
	// ������־�ļ�����С
	handle->log->pad_log_handle_fp->max_file_size = fileMaxSize;

	return 1;
}

/**
 * @brief	��Ȩ
 * ͨ����ȡ���ص�lic�ļ������������Ƿ���ȷ����Ч���Ƿ���ȷ
 * @return	�������ý�� 1-�ɹ� 0-ʧ��
 */
int RealtimeNetAuthentication(char * filePath)
{
	int ret = 0;

	//����ļ�Ϊ�գ����Ȩʧ��
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
 * @brief ���ļ��е����ݼ��ص�������
 *
 * @param fname �����ļ�ȫ·��
 * @param buf ���������ļ����ݵĻ�����
 * @param bufLen ��������С
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * ��ȡ�����ļ��е����ݣ����ļ������ݿ�����������
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
 * @brief ������յ�������DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
int RealtimeNetDoNormalMsg(RtNetHandle *handle, int reader, void * inMsg)
{
	int ret = 0;
	RealtimeNetData_Msg *msg = inMsg;
	// ���Ҷ�Ӧ��Topic
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

	// ���»����������ûص�����
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
 * @brief ������յ��ļ��DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
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
 * @brief ���ͼ�������Ӧ����Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
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
		printf("RealtimeNetSendMonitalResMsg:dds_write err[%s]��\n", dds_strretcode(-rc));
		return 0;
	}
	else
	{
		//printf("RealtimeNetMonitorSendThread:dds_write success��\n");
	}
	return 1;
}


/**
 * @brief ���ͼ��Ľڵ���Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
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
		printf("RealtimeNetSendMonitalNodeMsg:dds_write err[%s]��\n", dds_strretcode(-rc));
		return 0;
	}
	else
	{
		//printf("RealtimeNetSendMonitalNodeMsg:dds_write success��\n");
	}	   
	free(des);

	return 1;

}


int IsEndWithDotOne(const char* ip) {
	size_t len = strlen(ip);
	return (len >= 2 && ip[len - 2] == '.' && ip[len - 1] == '1');
}

/**
 * @brief ��ȡip��ַ
 *
 * @param  addr ip��ַ
 * @return ip��ַ
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
 * @brief ��ȡip��ַ���.���������
 *
 * @param  addr ip��ַ
 * @return .���������
 *
 *
 */
char getIpLastChar(const char *ip) 
{
	if (!ip) return -1;

	// �������һ����ŵ�λ��
	char *last_dot = strrchr(ip, '.');
	if (!last_dot || last_dot == ip + strlen(ip) - 1) {
		return -1;  // ��Ч��ʽ
	}

	// ��ȡ��ź������
	const char *octet_str = last_dot + 1;

	// ת��Ϊ����
	char *endptr;
	long num = strtol(octet_str, &endptr, 10);

	// ��֤ת�����
	if (*endptr != '\0' || num < 0 || num > 255) {
		return -1;  // ��Ч����
	}

	return (char)num;
}
