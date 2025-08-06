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
extern const char * 			c_topicNameReq;		// ���������topic��
extern const char * 			c_topicNameRes;		// ����Ӧ���topic��
extern const char * 			c_topicNameNode;	// ���սڵ��topic��


//ÿһ��topic��Ӧ�Ľṹ��
typedef struct ST_MoitorTopicItem_tag
{
	char *					topicName;				// ͨ��DDS����/������Ϣʹ�õ���TopicName
	dds_entity_t			topic;					// topic
	dds_entity_t			reader;					// reader
	void *					nextTopic;				// ָ����һ��topic
}MoitorTopicItem;

typedef struct ST_RtNetMonitorHandle_tag
{
	OnReceiverNodeFun		onReceiverNode;			// �ڵ���Ϣ�ص�����
	OnReceiverHeartBeatFun	onReceiverRes;			// ������Ϣ�ص�����
	OnReceiverDataFun		OnReceiverData;			// ҵ����Ϣ�Ļص�����
	int						timeout;				// ������Ϣ��Ƶ�ʣ���λ��

	dds_entity_t			participant;			// ������
	dds_entity_t			waitset;				// ��Ϣ��������
	ddsrt_mutex_t			lock;					// ��̬���topic������
	struct ddsrt_hh *		readerHash;				// �洢item�Ĺ�ϣhandle by reader
	MoitorTopicItem *		topicHead;				// ָ��topic�����ָ��

	dds_entity_t			topicReq;				// �������������topic
	dds_entity_t			topicRes;				// ��������Ӧ���topic
	dds_entity_t			topicNode;				// ���սڵ���Ϣ��topic

	dds_entity_t			writerReq;				// �������������ʵ��
	dds_entity_t			readerRes;				// ��������Ӧ���ʵ��
	dds_entity_t			readerNode;				// ���սڵ���Ϣ��ʵ��

	bool					exitFlg;				// �˳���־��֪ͨ�����߳��˳�
	bool					sendExitFlg;			// �����߳��˳��ɹ���־
	bool					recvExitFlg;			// �����߳��˳��ɹ���־

	ddsrt_thread_t			sendTid;				// �ڲ�������Ϣ���߳�ID
	ddsrt_thread_t			recvTid;				// �ڲ�������Ϣ���߳�ID
}RtNetMonitorHandle;

/*
 * @brief ��ʼ��handle
 *
 * @param handle �ڲ�ȫ�ֽṹ����id
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú�����ʼ��DDS���ã����������ߡ�
 */
int RealtimeNetMonitorInitHandle(RtNetMonitorHandle * handle);

/*
 * @brief ����DDS������
 *
 * @param ddsDominId dds��id
 * @param networkSegment  dds��ʼ��ʹ�õ�����ip
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú�����ʼ��DDS���ã����������ߡ�
 */
int RealtimeNetMonitorCreatePart(int ddsDominId, RtNetMonitorHandle * handle);

/**
 * @brief ����Topic�Ͳ�������Reader/Writer��
 *
 * @param handle ���������Topicӳ���б�
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú���Ϊÿ��Topic������Ӧ��Reader��Writer��������洢������С�
 */
int RealtimeNetMonitorCreateTopicAndOper(RtNetMonitorHandle * handle);

/**
 * @brief ��������������ע�ᵽWaitSet
 *
 * @param handle ���������Reader�б�
 * @return int �ɹ�����1
 *
 * �ú���Ϊÿ��Reader����ReadCondition��������ע�ᵽWaitSet�С�
 */
int RealtimeNetMonitorCreateCond(RtNetMonitorHandle *handle);

/**
 * @brief ������Ϣ�����߳�
 *
 * @param handle ���������Reader�б�
 * @return int �ɹ�����1
 *
 * �������Reader���򴴽�һ���߳����ڽ�����Ϣ��
 */
int RealtimeNetMonitorCreateThread(RtNetMonitorHandle *handle);

/**
 * @brief ��Ϣ�����̣߳����ڴ�����յ���DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetMonitorHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
int RealtimeNetMonitorRecvThread(void * inPara);

/**
 * @brief ��������Ӧ����Ϣ
 *
 * @param inPara ���������ָ��RtNetMonitorHandle
 * @param msg ���������ָ��Ӧ����Ϣ
 * @return 
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
void RealtimeNetMonitorDoResMsg(RtNetMonitorHandle *handle, void * msg);

/**
 * @brief ����ڵ�Ӧ����Ϣ
 *
 * @param inPara ���������ָ��RtNetMonitorHandle
 * @param msg ���������ָ��Ӧ����Ϣ
 * @return
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
void RealtimeNetMonitorDoNodeMsg(RtNetMonitorHandle *handle, void * msg);

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
void RealtimeNetMonitorDoDataMsg(RtNetMonitorHandle *handle, dds_attach_t triggered_reader, void * msg);

/**
 * @brief ��Ϣ�����̣߳����ڷ���DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetMonitorHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
int RealtimeNetMonitorSendThread(void * inPara);

/**
 * @brief ����Topic
 *
 * @param handle ���
 * @param reader ��ϣ����
 * @return int item
 *
 * ����ƫ�����ڹ�ϣ���в��Ҷ�Ӧ��Topic item��
 */
MoitorTopicItem * RealtimeNetMonitorFindTopicbyReader(RtNetMonitorHandle *handle, unsigned int reader);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_INNER_H
