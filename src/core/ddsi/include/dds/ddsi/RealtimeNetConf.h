#ifndef REALTIME_NET_CONF_H
#define REALTIME_NET_CONF_H

#include "dds/dds.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/ddsc/dds_public_qosdefs.h"


#if defined (__cplusplus)
extern "C" {
#endif
	//license�������
	typedef struct ST_licence_tag
	{
		char 						licencePath[128];		// license�����ļ�·��
	}licence;

	//ͨѶЭ���������
	typedef struct ST_commProtocol_tag
	{
		int			type;					// 0��udp�� 1��tcp�� 2��shm
	}commProtocol;
	
	//qos�������
	typedef struct ST_Qos_tag
	{
		short							durability_kind_valid;		// �־����Ƿ���Ч
		dds_durability_kind_t			durability_kind;			// �־���	dds_qset_durability
		short							presentation_valid;			// ˳���Ƿ���Ч
		dds_presentation_qospolicy_t	presentation;				// ˳��		dds_qset_presentation
		short							deadline_valid;				// ��ֹʱ���Ƿ���Ч
		int64_t							deadline;					// ��ֹʱ�� dds_qset_deadline
		short							ownership_kind_valid;		// ����Ȩ�Ƿ���Ч
		dds_ownership_kind_t			ownership_kind;				// ����Ȩ   dds_qset_ownership
		short							liveliness_valid;			// �����Ƿ���Ч
		dds_liveliness_qospolicy_t		liveliness;					// ����		dds_qset_liveliness
		short							time_based_filter_valid;	// ����ʱ��Ĺ������Ƿ���Ч
		dds_duration_t					minimum_separation;			// ����ʱ��Ĺ�����   dds_qset_time_based_filter
		short							partition_valid;			// �����Ƿ���Ч
		char *							partition;					// ����		dds_qset_partition1
		short							reliability_valid;			// �ɿ����Ƿ���Ч
		dds_reliability_qospolicy_t		reliability;				// �ɿ���	dds_qset_reliability
		short							destination_order_valid;	// Ŀ�ĵض����Ƿ���Ч
		dds_destination_order_qospolicy_t destination_order;		// Ŀ�ĵض���	dds_qset_destination_order
		short							history_valid;				// ��ʷ�����Ƿ���Ч
		dds_history_qospolicy_t			history;					// ��ʷ����	dds_qset_history
		short							resource_limits_valid;		// ��Դ�����Ƿ���Ч
		dds_resource_limits_qospolicy_t resource_limits;			// ��Դ����	dds_qset_resource_limits

	}Qos;

	//��־�������
	typedef struct ST_logging_tag
	{
		int							logLevel;
		char						logPath[128];		// ��־�ļ�·��
		int							maxFileSize;			//��־�ļ���С
	}logging;

	//��������
	typedef struct ST_RealtimeNetConf_tag
	{
		licence						lice;					// license
		commProtocol				comPro;					// ͨ��Э��
		Qos							qos;					// Qos
		logging						log;					// ��־
	}RealtimeNetConf;

/**
 * @brief ��ȡ����
 *
 * @param path �����ļ���ȫ·��
 * @param pConf ��ȡ����������Ϣ
 * @return int �ɹ�����1��ʧ�ܷ���0
 *
 * �������ļ��е����ü��ص�������Ϣ�ṹ���С������ڵ���Ĭ��ֵ
 */
int  RealtimeNetReadConf(char * path, RealtimeNetConf * pConf);


/**
 * @brief ����������Ϣ�ڴ�
 *
 * @param 
 * @param 
 * @return ָ��������Ϣ�Ľṹ��ָ��
 *
 * Ϊ������Ϣ�ṹ�������ڴ棬����ʼ��Ϊ0
 */
RealtimeNetConf *  RealtimeNetReadConf_malloc();

/**
 * @brief �ͷ�������Ϣ�ڴ�
 *
 * @param pConf ������Ϣָ��
 * @param
 * @return ��
 *
 * Ϊ������Ϣ�ṹ���ͷ��ڴ棬�ڲ���ָ��Ҳ�ͷ��ڴ�
 */
void  RealtimeNetReadConf_free(RealtimeNetConf * pConf);

/**
 * @brief �������ļ��е����ݼ��ص�������
 *
 * @param fname �����ļ�ȫ·��
 * @param buf ���������ļ����ݵĻ�����
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * ��ȡ�����ļ��е����ݣ������ڴ棬���ļ������ݿ������ڴ�
 */
int  RealtimeNetReadConf_fileToBuf(char * fname, char **buf);


/**
 * @brief ��ʼ��������Ϣ
 *
 * @param pConf ������Ϣ�ṹ��
 * @param 
 * @return 
 *
 * ��������Ϣ��Ĭ��ֵ
 */
void RealtimeNetReadConf_initConf(RealtimeNetConf * pConf);

/**
 * @brief ����QOS��Ϣ
 *
 * @param qos dds��qos�ṹ��
 * @param myQos ҵ�����qos�ṹ��
 * @return
 *
 * ��ҵ�����qos��Ϣ��ֵ��dds��qos�ṹ��
 */
void RealtimeNetReadConf_setQos(dds_qos_t * qos, Qos * myQos);

/**
 * @brief ������������ת��Ϊ������Ϣ�ṹ��
 *
 * @param buf �洢�������ݵĻ�����
 * @param pConf ������Ϣ�ṹ��
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * �ѻ�����������ת��Ϊ���ýṹ��
 */
int  RealtimeNetReadConf_bufToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief ��ӡ������Ϣ�ṹ��
 *
 * @param pConf ������Ϣ�ṹ��
 * @param 
 * @return 
 *
 * ��ӡ������Ϣ�ṹ��
 */
void RealtimeNetReadConf_printConf(RealtimeNetConf * pConf);

/**
 * @brief license��Ϣת��Ϊ�ṹ��
 *
 * @param buf ������
 * @param pConf ������Ϣ�ṹ��
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * license��Ϣת��Ϊ�ṹ��
 */
int  RealtimeNetReadConf_licenseToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief ͨѶ��Ϣת��Ϊ�ṹ��
 *
 * @param buf ������
 * @param pConf ������Ϣ�ṹ��
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * ͨѶ��Ϣת��Ϊ�ṹ��
 */
int  RealtimeNetReadConf_commToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief qos��Ϣת��Ϊ�ṹ��
 *
 * @param buf ������
 * @param pConf ������Ϣ�ṹ��
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * qosת��Ϊ�ṹ��
 */
int  RealtimeNetReadConf_QosToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief log��Ϣת��Ϊ�ṹ��
 *
 * @param buf ������
 * @param pConf ������Ϣ�ṹ��
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * logת��Ϊ�ṹ��
 */
int  RealtimeNetReadConf_loggingToConf(char *buf, RealtimeNetConf * pConf);

/**
 * @brief ��ȡxml�е���Ϣ
 *
 * @param buf ������
 * @param key xml�м������е��ֶ�
 * @param info �����ֶ����м��ֵ
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * ��ȡxml�е���Ϣ  
 * ʾ����
 *       buf����<�ֶ�>ֵ</�ֶ�> ���� 
 *       key�����ֶΡ���
 *       info����ֵ��
 */
int  RealtimeNetReadConf_getInfoFromBuf(char *buf, char *key, char **info);

/**
 * @brief ��ȡxml�е���Ϣ
 *
 * @param buf ������
 * @param key xml�м������е��ֶ�
 * @param info �����ֶ����м��ֵ
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * ��ȡxml�е���Ϣ
 * ʾ����
 *       buf����<�ֶ� ����=ֵ>����</�ֶ�> ����
 *       tag�����ֶΡ���
 *       attr�������ԡ���
 *       value����ֵ��
 */
int RealtimeNetReadConf_getAttrFromBuf2(char *buf, const char *tag, const char *attr, char **value);

/**
 * @brief ��ȡxml�е���Ϣ
 *
 * @param buf ������
 * @param key xml�м������е��ֶ�
 * @param info �����ֶ����м��ֵ
 * @return �������ý�� 1-�ɹ� 0-ʧ��
 *
 * ��ȡxml�е���Ϣ
 * ʾ����
 *       buf����<�ֶ�>ֵ</�ֶ�> ����
 *       key�����ֶΡ���
 *       info����ֵ��
 */
int  RealtimeNetReadConf_getInfoFromBuf3(char *buf, char *key, char* attr, char **info);

/**
 * @brief ����ǰ������÷���
 *
 * @param buf ������
 * @param 
 * @return 
 *
 * �ڻ�������ʼ�������˵����õķ��ţ�
 * ��Щ���Ű����ո񣬻س������ɣ�tab��
 */
void RealtimeNetReadConf_filterSymbols_begin(char ** buf);

/**
 * @brief ���˺�������÷���
 *
 * @param buf ������
 * @param
 * @return
 *
 * �ڻ�������������������˵����õķ��ţ�
 * ��Щ���Ű����ո񣬻س������ɣ�tab��
 */
void RealtimeNetReadConf_filterSymbols_end(char ** buf);

/**
 * @brief ת��ͨ��Э������
 *
 * @param buf ������
 * @param type ͨ��Э������
 * @return 
 *
 * ���ݻ��������ַ�����ת����ͨ��Э������
 */
void RealtimeNetReadConf_converCommProtocol(char * buf, int * type);

/**
 * @brief ת��durability����
 *
 * @param buf ������
 * @param type durability����
 * @return
 *
 * ���ݻ��������ַ�����ת����durability����
 */
void RealtimeNetReadConf_converDurability(char * buf, dds_durability_kind_t * type);

/**
 * @brief ת��presentation����
 *
 * @param buf ������
 * @param type presentation����
 * @return
 *
 * ���ݻ��������ַ�����ת����presentation����
 */
void RealtimeNetReadConf_converPresentation(char * buf, dds_presentation_qospolicy_t * type);

/**
 * @brief ת��access_scope����
 *
 * @param buf ������
 * @param type access_scope����
 * @return
 *
 * ���ݻ��������ַ�����ת����access_scope����
 */
void RealtimeNetReadConf_converAccess_scope(char * buf, dds_presentation_access_scope_kind_t * type);
	
/**
 * @brief ת��bool����
 *
 * @param buf ������
 * @param type bool����
 * @return
 *
 * ���ݻ��������ַ�����ת����bool����
 */
void RealtimeNetReadConf_converBool(char * buf, unsigned char * type);

/**
 * @brief ת��int����
 *
 * @param buf ������
 * @param type int����
 * @return
 *
 * ���ݻ��������ַ�����ת����int����
 */
void RealtimeNetReadConf_converInt(char * buf, int * type);

/**
 * @brief ת��longlong����
 *
 * @param buf ������
 * @param type longlong����
 * @return
 *
 * ���ݻ��������ַ�����ת����longlong����
 */
void RealtimeNetReadConf_converLongLong(char * buf, int64_t * type);

/**
 * @brief ת��ownership����
 *
 * @param buf ������
 * @param type ownership����
 * @return
 *
 * ���ݻ��������ַ�����ת����ownership����
 */
void RealtimeNetReadConf_converOwnership(char * buf, dds_ownership_kind_t * type);

/**
 * @brief ת��liveliness_qospolicy����
 *
 * @param buf ������
 * @param type liveliness_qospolicy����
 * @return
 *
 * ���ݻ��������ַ�����ת����liveliness_qospolicy����
 */
void RealtimeNetReadConf_converLiveliness(char * buf, dds_liveliness_qospolicy_t * type);

/**
 * @brief ת��liveliness_kind����
 *
 * @param buf ������
 * @param type liveliness_kind����
 * @return
 *
 * ���ݻ��������ַ�����ת����liveliness_kind����
 */
void RealtimeNetReadConf_converLiveliness_kind(char * buf, dds_liveliness_kind_t * type);

/**
 * @brief ת��reliability_qospolicy����
 *
 * @param buf ������
 * @param type reliability_qospolicy����
 * @return
 *
 * ���ݻ��������ַ�����ת����reliability_qospolicy����
 */
void RealtimeNetReadConf_converReliability(char * buf, dds_reliability_qospolicy_t * type);

/**
 * @brief ת��reliability_kind����
 *
 * @param buf ������
 * @param type reliability_kind����
 * @return
 *
 * ���ݻ��������ַ�����ת����reliability_kind����
 */
void RealtimeNetReadConf_converReliability_kind(char * buf, dds_reliability_kind_t * type);

/**
 * @brief ת��destination_order_kind����
 *
 * @param buf ������
 * @param type destination_order_kind����
 * @return
 *
 * ���ݻ��������ַ�����ת����destination_order_kind����
 */
void RealtimeNetReadConf_converDestination_order(char * buf, dds_destination_order_kind_t * type);
	
/**
 * @brief ת��history_qospolicy����
 *
 * @param buf ������
 * @param type history_qospolicy����
 * @return
 *
 * ���ݻ��������ַ�����ת����history_qospolicy����
 */
void RealtimeNetReadConf_converHistory(char * buf, dds_history_qospolicy_t * type);

/**
 * @brief ת��history_kind����
 *
 * @param buf ������
 * @param type history_kind����
 * @return
 *
 * ���ݻ��������ַ�����ת����history_kind����
 */
void RealtimeNetReadConf_converHistory_kind(char * buf, dds_history_kind_t * type);

/**
 * @brief ת��converResource_limits����
 *
 * @param buf ������
 * @param type converResource_limits����
 * @return
 *
 * ���ݻ��������ַ�����ת����converResource_limits����
 */
void RealtimeNetReadConf_converResource_limits(char * buf, dds_resource_limits_qospolicy_t * type);

/**
 * @brief ת��converLogLevel����
 *
 * @param buf ������
 * @param type converLogLevel����
 * @return
 *
 * ���ݻ��������ַ�����ת����converLogLevel����
 */
void RealtimeNetReadConf_converLogLevel(char * buf, int * type);

/**
 * @brief ���ַ���ת��Ϊshort
 *
 * @param val �ַ���
 * @return	short
 */
short str_to_short(const char *val);
	
#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_CONF_H