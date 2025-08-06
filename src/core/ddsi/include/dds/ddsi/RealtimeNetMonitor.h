#ifndef REALTIME_NET_MIDDLEWARE_MONITOR_H
#define REALTIME_NET_MIDDLEWARE_MONITOR_H

#include "dds/dds.h"
#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif
	
#pragma pack(1) /*1�ֽڶ���*/

enum MONITOR_TOPIC_TYPE
{
	MONITOR_TOPIC_TYPE_READ = 1,
	MONITOR_TOPIC_TYPE_WRITE
};

typedef struct ST_MonitorTopicItem_tag
{
	char*		topicName;				// ͨ��DDS����/������Ϣʹ�õ���TopicName
	int			size;					// TopicName��Ӧ���ݵĻ�������С
	int			status;					// ��TopicName�Ƕ�����д�����ֶ���ҪΪ�˼��ݷ����ڴ���
}MonitorTopicItem;

typedef struct ST_NodeInfo_tag
{
	const char*				networkSegment;		// ip
	int						processId;			// ����ID
	int						threadId;			// �߳�ID
	MonitorTopicItem*		topicMapList;		// topic��Ϣ�б�
	int						topicMapListLength;	// ӳ���ĳ���
}RtNodeInfo;


#pragma pack()/*��ԭĬ�϶���*/

// ���庯��ָ�룬�ѽڵ���Ϣ���ݸ�ҵ����Ҫҵ�����Ϣ������ȥ��
typedef void(*OnReceiverNodeFun)(RtNodeInfo * node);

// ���庯��ָ�룬����ͨ���ûص�������ע��topicName������
typedef void(*OnReceiverHeartBeatFun)(char ip, int processId, int threadId);

// ���庯��ָ�룬����ͨ���ûص�������ע��topicName������
typedef void(*OnReceiverDataFun)(char * topicName, void * pBuffer, unsigned int size, char ip, int processId, int threadId);

/**
 * @brief	��صĳ�ʼ��
 *
 * �����ⲿ������ô����Ĳ��������м�صĳ�ʼ��
 *
 * @param	ddsDominId		dds��id
 * @param	timeout			������ʱ����
 * @param	onReceiverNode	�ڵ���Ϣ�Ļص�
 * @param	onReceiverHeartBeat	������Ϣ�Ļص�
 * @return					���ؾ��
 */
DDS_EXPORT  void * MonitorInit(int ddsDominId, int timeout, OnReceiverNodeFun onReceiverNode, OnReceiverHeartBeatFun onReceiverHeartBeat, OnReceiverDataFun OnReceiverData);

/**
 * @brief	����topic
 *
 * �������ӵ�topic,������и�topic�����ݣ�ͨ���ص����ظ�ҵ���
 *
 * @param	pContext		��ʼ��ʱ�����صľ��
 * @param	topicName		topic��
 * @return			�������ý�� 1-�ɹ� 0-ʧ��
 */
DDS_EXPORT  int MonitorAddTopic(void* pContext, char * topicName);


/**
 * @brief	ͨ���м�����ͷ�
 *
 *
 *
 * @param	pContext ��ʼ��ʱ�����صľ��
 * @return	null
 */
DDS_EXPORT void  MonitortFree(void* pContext);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_H
