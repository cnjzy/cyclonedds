#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#elif __linux__
#include <cpuid.h>
#endif

#include "dds/ddsi/RealtimeNetLicense.h"

const char * c_encode_key = "JiaZiYing";

/**
 * @brief 获取cpu序列号
 * @param buffer    序列号字符串
 * @param size      序列号字符串长度
 * @return
 */
#ifdef _WIN32
void get_cpu_serial(char* buffer, size_t size) {
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 1);  // 调用 CPUID 指令获取处理器信息
    snprintf(buffer, size, "%08X%08X", cpuInfo[3], cpuInfo[0]);
}
#elif __linux__
void get_cpu_serial(char* buffer, size_t size) {
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx); // 调用 CPUID 指令获取处理器信息
    snprintf(buffer, size, "%08X%08X", edx, eax);
}
#endif


/**
 * @brief 使用XOR算法加密/解密字符串
 * @param input 输入字符串
 * @param input_len 输入字符串有效长度
 * @param key 密钥
 * @param key 密钥有效长度
 * @param value 加密结果
 * @return
 */
void xor_crypt(const char* input, int input_len, const char* key, int key_len, char * value)
{
    // 逐个字符进行XOR运算
    for (size_t i = 0; i < input_len; i++) {
        value[i] = input[i] ^ key[i % key_len];
    }
}


/**
 * @brief	获取code码
 *
 * 根据主机的硬件配置，生成当前主机的code码
 *
 * @param	reqCode 指针地址，指向code字符串。使用完毕后，需要释放。
 * @return	null
 */
void rnml_getReqCode(char reqCode[16+1])
{
    //定义临时变量
    char serial[16+1];
    //初始化
    memset(serial, 0, sizeof(serial));
    //获取序列号
    get_cpu_serial(serial, sizeof(serial));
    //拷贝序列号
    strncpy(reqCode, serial, 16);
    //添加结束符
    reqCode[16] = '\0';
}

/**
 * @brief	产生license信息
 *
 * @param	reqCode  code码,格式为16个字节的可见字符串
 * @param	time     有效期，格式为年月日，例如：20251010
 * @param	lic      生成的license字符串。有效长度32字节。
 * @return	null
 */
void rnml_greateLic(char reqCode[16+1], char date[8+1], char lic[32+1])
{
    //定义变量
    char licBuf[32+1];
    int now;
    char timeBuf[8+1];

    //初始哈
    memset(licBuf, 0, sizeof(licBuf));
    memset(timeBuf, 0, sizeof(timeBuf));

    //获取当前时间作为动态的key
    now = (int)time(NULL);
    snprintf(timeBuf, 9, "%08X", now);

    //动态key加密成二进制key
    xor_crypt(timeBuf, 8, c_encode_key, strlen(c_encode_key), timeBuf);

    //拷贝code和过期时间

    strncpy(licBuf, reqCode, 16);
    strncpy(licBuf+16, date, 8);

    //对code和过期时间加密
    xor_crypt(licBuf, 24, timeBuf, 8, lic);

    //二进制key也放进去，解码时使用。
    memcpy(&(lic[24]), timeBuf, 8);
}

/**
 * @brief	获取当前的年月日
 *
 * @param	date[9]  格式为年月日，示例：20250801
 * @return	null
 */
void rnml_getDate(char date[8+1])
{
    time_t now = time(NULL);               // 获取当前时间戳
    struct tm *local = localtime(&now);    // 转换为本地时间结构体

    int year  = local->tm_year + 1900;     // 年份需加1900
    int month = local->tm_mon + 1;         // 月份需加1（0~11 → 1~12）
    int day   = local->tm_mday;

    //把时间写到date
    snprintf(date, 9, "%04d%02d%02d", year, month, day);
}

/**
 * @brief	验证license
 *
 * 根据license字符串，鉴权。如果硬件信息正确，并且在有效期内，则返回true
 *
 * @param	licPath license文件的全路径
 * @return	1 鉴权成功， 0 鉴权失败
 */
int rnml_startVerify(char lic[32+1])
{
    //定义变量
    char buf[33];
    char licBuf[16+1];
    char timeBuf[8+1];
    char nowBuf[8+1];
    int ret;

    //初始化
    memset(buf, 0, sizeof(buf));
    memset(licBuf, 0, sizeof(licBuf));
    memset(timeBuf, 0, sizeof(timeBuf));
    memset(nowBuf, 0, sizeof(nowBuf));

    //拷贝出解码用key
    memcpy(timeBuf, lic+24, 8);

    //用key解码
    xor_crypt(lic, 24, timeBuf, 8, buf);

    //获取主机code
    rnml_getReqCode(licBuf);
    //对比code是否相同
    ret = strncmp(licBuf, buf, 16);
    if(ret)
        return 0;

    //获取当前时间
    rnml_getDate(nowBuf);
    //判断是否过了有效期
    ret = strncmp(timeBuf, nowBuf, 8);
    //没过有效期返回成功
    if(ret >= 0)
        return 1;

    //过了有效期，返回失败
    return 0;

}



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
void rnml_parseLic(char lic[32+1], char reqCode[16+1], char date[8+1])
{
    //定义变量
    char buf[24+1];
    char timeBuf[8+1];

    //初始化
    memset(buf, 0, sizeof(buf));
    memset(timeBuf, 0, sizeof(timeBuf));

    //拷贝出解码用key
    memcpy(timeBuf, lic+24, 8);

    //用key解码
    xor_crypt(lic, 24, timeBuf, 8, buf);

    //拷贝code
    strncpy(reqCode, buf, 16);
    reqCode[16] = '\0';

    //拷贝日期
    strncpy(date, buf+16, 8);
    date[8] = '\0';
}
