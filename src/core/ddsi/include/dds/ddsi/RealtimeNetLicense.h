#ifndef REALTIME_NET_MIDDLEWARE_LICENSE_H
#define REALTIME_NET_MIDDLEWARE_LICENSE_H


#if defined (__cplusplus)
extern "C" {
#endif

/**
 * @brief	获取code码
 *
 * 根据主机的硬件配置，生成当前主机的code码
 *
 * @param	reqCode 指针地址，指向code字符串,有效长度16字节
 * @return	null
 */
void rnml_getReqCode(char reqCode[16+1]);


/**
 * @brief	产生license信息
 *
 * 
 *
 * @param	reqCode  code码,格式为16个字节的可见字符串
 * @param	time     有效期，格式为年月日，例如：20251010
 * @param	lic      生成的license字符串。有效长度32字节。
 * @return	null
 */
void rnml_greateLic(char reqCode[16+1], char date[8+1], char lic[32+1]);


/**
 * @brief	验证license
 *
 * 根据license字符串，鉴权。如果硬件信息正确，并且在有效期内，则返回true
 *
 * @param	lic 需要鉴权的字符串，有效长度32字节
 * @return	1 鉴权成功， 0 鉴权失败
 */
int rnml_startVerify(char lic[32+1]);

/**
 * @brief	解析license
 *
 * 根据license字符串，解析。
 *
 * @param	lic 需要鉴权的字符串，有效长度32字节
 * @param	reqCode  code码,格式为16个字节的可见字符串
 * @param	time     有效期，格式为年月日，例如：20251010
 * @return
 */
void rnml_parseLic(char lic[32+1], char reqCode[16+1], char date[8+1]);

#ifdef __cplusplus
}
#endif

#endif // !REALTIME_NET_MIDDLEWARE_H
