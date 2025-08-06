#ifndef REALTIME_NET_MIDDLEWARE_INNER_H
#define REALTIME_NET_MIDDLEWARE_INNER_H

#include "dds/ddsrt/threads.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/time.h"

#include "RealtimeNetMiddleware.h"

#ifdef __cplusplus
extern "C" {
#endif		

#pragma pack(1) /*1�ֽڶ���*/

typedef struct ST_TopicMapInnerItem_tag
{
	char *		topicName;				// ͨ��DDS����/������Ϣʹ�õ���TopicName
	int			offset;					// ԭ�����ڴ�����Offset
	int			size;					// TopicName��Ӧ���ݵĻ�������С
	int			status;					// ��TopicName�Ƕ�����д�����ֶ���ҪΪ�˼��ݷ����ڴ���
	dds_entity_t topic;
	dds_entity_t operator;				//writer or reader
	char *		buf;					//�洢���һ�����ݵĻ�����
	int         num;					//��ǰitem���������
}TopicMapInnerItem;

typedef struct ST_RtNetHandle_tag
{
	int					ddsDominId;			// dds��id
	char *				networkSegment;		// dds��ʼ��ʹ�õ�����ip���������ж������ʱ����Ҫȷ�������ε��豸���յ�dds��Ϣ��������ֻ�е�������ʱ��Ϊ��
	TopicMapInnerItem *	topicMapList;		// ddsע���topic��Ϣ�б���ΪҪ���ݷ����ڴ�����offset�����Գ�ʼ��ʱ����TopicName��Offset��ӳ���
	int					topicMapListLength;	// ӳ���ĳ���
	OnReceiverDataFun	onReceiverTopic;	// ������Ϣ�Ļص���ͨ������ָ��ʵ�֡��ⲿ�����ʼ���ṹ��ʱ�����Դ���ñ����������ⲿ����ͨ���ص��첽�������ݡ�����ⲿ���գ�
	int					timeout;			// ������Ϣ�ĳ�ʱʱ�䣬��λ��

	dds_entity_t		participant;		//
	dds_entity_t		waitset;			//

	ddsrt_thread_t		tid;				//�ڲ�������Ϣ���߳�ID
	bool				exitFlg;			//�˳���־
	struct ddsrt_hh *	typecache;			//�洢item�Ĺ�ϣhandle
	ddsrt_mutex_t		lock;				//�ڲ����ݿ�����
}RtNetHandle;


#pragma pack()/*��ԭĬ�϶���*/

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
