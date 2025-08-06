#ifndef REALTIME_NET_MIDDLEWARE_INNER_H
#define REALTIME_NET_MIDDLEWARE_INNER_H

#include "dds/ddsrt/threads.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/time.h"
#include "dds/ddsi/ddsi_config.h"

#include "os_defs.h"

#include "RealtimeNetMiddleware.h"
#include "RealtimeNetConf.h"

#if defined (__cplusplus)
extern "C" {
#endif	

#pragma pack(1) /*1�ֽڶ���*/

typedef struct ST_LogSetting_tag
{
	/* ��־��Ϣ */
	log_handle* pad_log_handle_fp;
	os_log_attr_t log_attr;
} LogSetting;


//ÿһ��topic��Ӧ�Ľṹ��
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
	bool		haveDataFlg;			//�Ƿ�������
}TopicMapInnerItem;

//ÿ��ʾ��participanet��Ӧ�������ṹ��
typedef struct ST_RtNetHandle_tag
{
	int					ddsDominId;			// dds��id
	char *				networkSegment;		// dds��ʼ��ʹ�õ�����ip���������ж������ʱ����Ҫȷ�������ε��豸���յ�dds��Ϣ��������ֻ�е�������ʱ��Ϊ��
	TopicMapInnerItem *	topicMapList;		// ddsע���topic��Ϣ�б���ΪҪ���ݷ����ڴ�����offset�����Գ�ʼ��ʱ����TopicName��Offset��ӳ���
	int					topicMapListLength;	// ӳ���ĳ���
	OnReceiverDataFun	onReceiverTopic;	// ������Ϣ�Ļص���ͨ������ָ��ʵ�֡��ⲿ�����ʼ���ṹ��ʱ�����Դ���ñ����������ⲿ����ͨ���ص��첽�������ݡ�����ⲿ���գ�
	int					timeout;			// ������Ϣ�ĳ�ʱʱ�䣬��λ��

	dds_entity_t		participant;		// ������
	dds_entity_t		waitset;			// ��Ϣ��������

	dds_entity_t		topicReq;			// �������������topic
	dds_entity_t		topicRes;			// ��������Ӧ���topic
	dds_entity_t		topicNode;			// ���սڵ���Ϣ��topic
	dds_entity_t		readerReq;			// �������������ʵ��
	dds_entity_t		writerRes;			// ��������Ӧ���ʵ��
	dds_entity_t		writerNode;			// ���սڵ���Ϣ��ʵ��
	int					seq;				// ���������ID,�緢���仯���·���node��Ϣ

	ddsrt_thread_t		tid;				// �ڲ�������Ϣ���߳�ID
	bool				exitFlg;			// �˳���־
	bool				recvThreadExitFlg;	// �����߳��˳���־
	struct ddsrt_hh *	offsetReadHash;		// �洢item�Ĺ�ϣhandle by  Read offset
	struct ddsrt_hh *	offsetWriteHash;	// �洢item�Ĺ�ϣhandle by  Write offset
	struct ddsrt_hh *	readerHash;			// �洢item�Ĺ�ϣhandle by reader

	ddsrt_mutex_t		lock;				// �ڲ����ݿ�����
	struct ddsi_config	raw;				// dds��������Ϣ
	int					processId;			// ����id
	int					threadId;			// �߳�id
	char 				ipAddr[32];			// ip��ַ
	char				ip;					// ip��ַ�����һ��ֵ
	LogSetting *		log;				// ��־���
	RealtimeNetConf *	pConf;				// qos����
	dds_qos_t *			qos;				// dds�ڲ�qos

}RtNetHandle;


#pragma pack()/*��ԭĬ�϶���*/

/**
 * @brief ��ʼ��������������õ������
 *
 * @param initParam ��ʼ����������������Ρ�Topicӳ���б����Ϣ
 * @param handle ��������ڴ洢��ʼ����Ĳ���
 *
 * �ú�����`initParam`�еĲ������Ƶ�`handle`�У��������Ҫ���ڴ档
 * ���δ���ó�ʱʱ�䣬��Ĭ������Ϊ1Сʱ��
 */
void RealtimeNetSetParam(RtNetInitParam *initParam, RtNetHandle * handle);


/**
 * @brief ��ʼ����ϣ�����ڴ洢TopicMapInnerItem
 *
 * @param handle ���������Topicӳ���б�
 * @return int �ɹ�����1
 *
 * �ú�������һ����ϣ������Topicӳ���б��е�ÿ��Ԫ����ӵ���ϣ���С�
 */
int RealtimeNetInitHash(RtNetHandle *handle);

/**
 * @brief ����DDS������
 *
 * @param handle ����������������Ϣ
 * @param myQos  Qos
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú�����ʼ��DDS���ã����������ߣ�������洢������С�
 */
int RealtimeNetCreatePart(RtNetHandle *handle, Qos * myQos);

/**
 * @brief ����Topic�Ͳ�������Reader/Writer��
 *
 * @param handle ���������Topicӳ���б�
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú���Ϊÿ��Topic������Ӧ��Reader��Writer��������洢������С�
 */
int RealtimeNetCreateTopicAndOper(RtNetHandle *handle);

/**
 * @brief ����Topic�Ͳ�������Reader/Writer��
 *
 * @param handle ���������Topicӳ���б�
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú���Ϊÿ��Topic������Ӧ��Reader��Writer��������洢������С�
 */
int RealtimeNetCreateNormalTopicAndOper(RtNetHandle *handle);

/**
 * @brief ����Topic�Ͳ�������Reader/Writer��
 *
 * @param handle ���������Topicӳ���б�
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �ú���Ϊÿ��Topic������Ӧ��Reader��Writer��������洢������С�
 */
int RealtimeNetCreatMonitorTopicAndOper(RtNetHandle *handle);

/**
 * @brief ��������������ע�ᵽWaitSet
 *
 * @param handle ���������Reader�б�
 * @return int �ɹ�����1
 *
 * �ú���Ϊÿ��Reader����ReadCondition��������ע�ᵽWaitSet�С�
 */
int RealtimeNetCreateCond(RtNetHandle *handle);

/**
 * @brief ��Ϣ�����̣߳����ڴ�����յ���DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
int RealtimeNetMsgThread(void * inPara);


/**
 * @brief ������Ϣ�����߳�
 *
 * @param handle ���������Reader�б�
 * @return int �ɹ�����1
 *
 * �������Reader���򴴽�һ���߳����ڽ�����Ϣ��
 */
int RealtimeNetCreateThread(RtNetHandle *handle);

/**
 * @brief ����Topic by read offset
 *
 * @param handle ���
 * @param offset ƫ����
 * @return int �ɹ�����Topic������ʧ�ܷ���-1
 *
 * ����ƫ�����ڹ�ϣ���в��Ҷ�Ӧ��Topic��
 */
int RealtimeNetFindTopicbyReadOffset(RtNetHandle *handle, unsigned int offset);

/**
 * @brief ����Topic by write offset
 *
 * @param handle ���
 * @param offset ƫ����
 * @return int �ɹ�����Topic������ʧ�ܷ���-1
 *
 * ����ƫ�����ڹ�ϣ���в��Ҷ�Ӧ��Topic��
 */
int RealtimeNetFindTopicbyWriteOffset(RtNetHandle *handle, unsigned int offset);

/**
 * @brief ����Topic by reader
 *
 * @param handle ���
 * @param reader ��ϣ����
 * @return int �ɹ�����Topic������ʧ�ܷ���-1
 *
 * ����ƫ�����ڹ�ϣ���в��Ҷ�Ӧ��Topic��
 */
int RealtimeNetFindTopicbyReader(RtNetHandle *handle, unsigned int reader);


/**
 * @brief	��Ȩ
 *
 * @return	�������ý�� 1-�ɹ� 0-ʧ��
 */
int RealtimeNetAuthentication(char * filePath);

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
int  RealtimeNetReadFile(char * fname, char *buf, int bufLen);

/**
 * @brief ������յ�������DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
int RealtimeNetDoNormalMsg(RtNetHandle *handle, int reader, void * msg);

/**
 * @brief ������յ��ļ��DDS��Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * ���̴߳�WaitSet�еȴ�������Reader����ȡ��Ϣ�����ûص���������
 */
int RealtimeNetDoMonitalMsg(RtNetHandle *handle, void * msg);


/**
 * @brief ���ͼ�������Ӧ����Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * 
 */
int RealtimeNetSendMonitalResMsg(RtNetHandle *handle);


/**
 * @brief ���ͼ��Ľڵ���Ϣ
 *
 * @param inPara ���������ָ��RtNetHandle
 * @return int �ɹ�����1
 *
 * 
 */
int RealtimeNetSendMonitalNodeMsg(RtNetHandle *handle);

/**
 * @brief ��ȡip��ַ
 *
 * @param addr ip��ַ
 * @return 
 *
 *
 */
void get_local_ips(char * addr);

/**
 * @brief ��ȡip��ַ���.���������
 *
 * @param  addr ip��ַ
 * @return .���������
 *
 *
 */
char getIpLastChar(const char *ip);

/**
 * @brief	��־��ʼ��
 *
 * @param handle ���
 * @return	�������ý�� 1-�ɹ� 0-ʧ��
 */
int LogInit(RtNetHandle *handle, os_log_level level);

/**
 * @brief	������־����
 *
 *
 * @param handle ���
 * @param	level	��־����
 * @return	�������ý�� 1-�ɹ� 0-ʧ��
 */
int setLogLevel(RtNetHandle *handle, os_log_level level);

/**
 * @brief	������־�ļ�
 *
 * @param handle ���
 * @param	filePath	��־����
 * @param	fileName	��־����
 * @param	fileMaxSize	��־�ļ�����С���ֽڣ�
 * @return	�������ý�� 1-�ɹ� 0-ʧ��
 */
int setLogFile(RtNetHandle *handle, char filePath[256], char fileName[256], int fileMaxSize);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_INNER_H
