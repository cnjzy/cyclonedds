#ifndef REALTIME_NET_MIDDLEWARE_H
#define REALTIME_NET_MIDDLEWARE_H

#ifdef __cplusplus
extern "C" {
#endif


// ���庯��ָ�룬����ͨ���ûص�������ע��topicName������
typedef void (*OnReceiverDataFun)(char* topicName,unsigned int offset,void* pBuffer,unsigned int size);

#pragma pack(1) /*1�ֽڶ���*/

enum TOPIC_TYPE
{
	TOPIC_TYPE_READ = 1,
	TOPIC_TYPE_WRITE
};

typedef struct ST_TopicMapItem_tag
{
	char*		topicName;				// ͨ��DDS����/������Ϣʹ�õ���TopicName
	int			offset;					// ԭ�����ڴ�����Offset
	int			size;					// TopicName��Ӧ���ݵĻ�������С
	int			status;					// ��TopicName�Ƕ�����д�����ֶ���ҪΪ�˼��ݷ����ڴ���
}TopicMapItem;

typedef struct ST_RtNetInitParam_tag
{
	int					ddsDominId;			// dds��id
	const char*			networkSegment;		// dds��ʼ��ʹ�õ�����ip���������ж������ʱ����Ҫȷ�������ε��豸���յ�dds��Ϣ��������ֻ�е�������ʱ��Ϊ��
	TopicMapItem*		topicMapList;		// ddsע���topic��Ϣ�б���ΪҪ���ݷ����ڴ�����offset�����Գ�ʼ��ʱ����TopicName��Offset��ӳ���
	int					topicMapListLength;	// ӳ���ĳ���
	OnReceiverDataFun	onReceiverTopic;	// ������Ϣ�Ļص���ͨ������ָ��ʵ�֡��ⲿ�����ʼ���ṹ��ʱ�����Դ���ñ����������ⲿ����ͨ���ص��첽�������ݡ�����ⲿ���գ�
	int					timeout;			// ������Ϣ�ĳ�ʱʱ�䣬��λ��
}RtNetInitParam;


#pragma pack()/*��ԭĬ�϶���*/


/**
 * @brief	ͨ���м���ĳ�ʼ��
 *
 * �����ⲿ������ô����Ĳ����������м���ĳ�ʼ��
 *
 * @param	initParam �ⲿ���ݵĳ�ʼ�������ṹ��
 * @return	DDS�ڲ����ﷵ�صĶ���ָ�룬�����ڵ���RealtimeNetRead��RealtimeNetWrite����ʱʹ��
 */
void*	RealtimeNetInit(RtNetInitParam initParam);

/**
 * @brief	��ȡ����
 *
 * ���ݳ�ʼ��ʱ���صĶ����Լ�offsetֵ�������ݵĶ�ȡ
 *
 * @param pContext	����RealtimeInitʱ���صĶ���ָ��
 * @param offset	ԭ�����ڴ���ʹ�õ�offset��������Ҫ����TopicName��Offset��ӳ���ϵ����ȡTopicName���յ�������
 * @param pBuffer	�������ݻ�����
 * @param pBuffer	�������ݻ��峤��
 * @return			�������ý�� 1-�ɹ� 0-ʧ��
 */
int		RealtimeNetRead(void* pContext, unsigned int offset, void* pBuffer, unsigned int size);

/**
 * @brief	д������
 *
 * ���ݳ�ʼ��ʱ���صĶ����Լ�offsetֵ�������ݵ�д��
 *
 * @param pContext	����RealtimeInitʱ���صĶ���ָ��
 * @param offset	ԭ�����ڴ���ʹ�õ�offset��������Ҫ����TopicName��Offset��ӳ���ϵ����TopicName��������
 * @param pBuffer	�������ݻ�����
 * @param pBuffer	�������ݻ��峤��
 * @return			�������ý�� 1-�ɹ� 0-ʧ��
 */
int		RealtimeNetWrite(void* pContext,unsigned int offset,void* pBuffer,unsigned int size);

/**
 * @brief	ͨ���м�����ͷ�
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
