#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include<windows.h>

//#include "dds/ddsi/RealtimeNetMiddleware.h"

#include "RealtimeNetMiddleware.h"

/* An array of one message (aka sample in dds terms) will be used. */
#define MAX_SAMPLES 1
void recvMsg(char* topicName, unsigned int offset, void* pBuffer, unsigned int size)
{
	char buf[256];
	memset(buf, 0, sizeof(buf));
	memcpy(buf, pBuffer, size);
	printf("callback recvMsg: topicName[%s], offset[%d], pBuffer[%s], size[%d]\n", topicName, offset, (char *)buf, size);
}

int main (int argc, char ** argv)
{
  TopicMapItem  topicMapList[3];
  memset(&topicMapList, 0, sizeof(topicMapList));
  topicMapList[0].topicName = "dds_msg1";
  topicMapList[0].offset = 0;
  topicMapList[0].size = 6;
  topicMapList[0].status = TOPIC_TYPE_READ;
  topicMapList[1].topicName = "dds_msg2";
  topicMapList[1].offset = 100;
  topicMapList[1].size = 10;
  topicMapList[1].status = TOPIC_TYPE_READ;
  topicMapList[2].topicName = "dds_msg3";
  topicMapList[2].offset = 256;
  topicMapList[2].size = 16;
  topicMapList[2].status = TOPIC_TYPE_READ;

  RtNetInitParam initParam;
  memset(&initParam, 0, sizeof(initParam));
  initParam.ddsDominId = 0xffffffffu;
  initParam.networkSegment = NULL;
  initParam.topicMapList = &topicMapList[0];
  initParam.topicMapListLength = 3;
  initParam.onReceiverTopic = &recvMsg;
  initParam.networkSegment = "127.0.0.1";
  initParam.timeout = 3600;
  void *handle;

  handle = RealtimeNetInit(initParam);

  printf ("\n=== [Subscriber] Waiting message ...\n");
  fflush (stdout);

  /* Polling sleep. */
  Sleep(30000);


  char buf[33];
  memset(buf, 0, sizeof(buf));
  RealtimeNetRead(handle, 0, buf, 6);
  printf("readmsg offset[0]'s msg is :%s\n", buf);

  RealtimeNetRead(handle, 100, buf, 10);
  printf("readmsg offset[100]'s msg is :%s\n", buf);

  RealtimeNetRead(handle, 256, buf, 16);
  printf("readmsg offset[256]'s msg is :%s\n", buf);

  /* Polling sleep. */
  printf("\n 60秒后退出.\n");
  Sleep(60000);
  RealtimeNetFree(handle);
  printf("\n 已退出.30秒后关闭窗口\n");
  Sleep(30000);
  return EXIT_SUCCESS;
}
