#ifndef REALTIME_NET_MIDDLEWARE_LICENSE_H
#define REALTIME_NET_MIDDLEWARE_LICENSE_H


#if defined (__cplusplus)
extern "C" {
#endif

/**
 * @brief	��ȡcode��
 *
 * ����������Ӳ�����ã����ɵ�ǰ������code��
 *
 * @param	reqCode ָ���ַ��ָ��code�ַ���,��Ч����16�ֽ�
 * @return	null
 */
void rnml_getReqCode(char reqCode[16+1]);


/**
 * @brief	����license��Ϣ
 *
 * 
 *
 * @param	reqCode  code��,��ʽΪ16���ֽڵĿɼ��ַ���
 * @param	time     ��Ч�ڣ���ʽΪ�����գ����磺20251010
 * @param	lic      ���ɵ�license�ַ�������Ч����32�ֽڡ�
 * @return	null
 */
void rnml_greateLic(char reqCode[16+1], char date[8+1], char lic[32+1]);


/**
 * @brief	��֤license
 *
 * ����license�ַ�������Ȩ�����Ӳ����Ϣ��ȷ����������Ч���ڣ��򷵻�true
 *
 * @param	lic ��Ҫ��Ȩ���ַ�������Ч����32�ֽ�
 * @return	1 ��Ȩ�ɹ��� 0 ��Ȩʧ��
 */
int rnml_startVerify(char lic[32+1]);

/**
 * @brief	����license
 *
 * ����license�ַ�����������
 *
 * @param	lic ��Ҫ��Ȩ���ַ�������Ч����32�ֽ�
 * @param	reqCode  code��,��ʽΪ16���ֽڵĿɼ��ַ���
 * @param	time     ��Ч�ڣ���ʽΪ�����գ����磺20251010
 * @return
 */
void rnml_parseLic(char lic[32+1], char reqCode[16+1], char date[8+1]);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_H
