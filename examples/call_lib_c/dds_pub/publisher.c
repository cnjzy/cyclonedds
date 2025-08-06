
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include<windows.h>

#include "RealtimeNetMiddleware.h"

int main (int argc, char ** argv)
{

  int rc;
  int status = 0;

  TopicMapItem  topicMapList[3];
  memset(&topicMapList, 0, sizeof(topicMapList));
  topicMapList[0].topicName = "dds_msg1";
  topicMapList[0].offset = 0;
  topicMapList[0].size = 6;
  topicMapList[0].status = TOPIC_TYPE_WRITE;
  topicMapList[1].topicName = "dds_msg2";
  topicMapList[1].offset = 100;
  topicMapList[1].size = 10;
  topicMapList[1].status = TOPIC_TYPE_WRITE;
  topicMapList[2].topicName = "dds_msg3";
  topicMapList[2].offset = 256;
  topicMapList[2].size = 16;
  topicMapList[2].status = TOPIC_TYPE_WRITE;

  RtNetInitParam initParam;
  memset(&initParam, 0, sizeof(initParam));
  initParam.ddsDominId = 0xffffffffu;
  initParam.topicMapList = &topicMapList[0];
  initParam.topicMapListLength = 3;
  initParam.onReceiverTopic = NULL;
  initParam.networkSegment = "127.0.0.1";
  initParam.timeout = 3600;
  void *handle;
  handle = RealtimeNetInit(initParam);
  
  /* Create a message to write. */
  printf("=== [Publisher]  Writing : \n");
  fflush(stdout);
  int i = 0;
  while(1)
  { 

	  Sleep(10000);
	  char * message1 = "Hello1";
	  RealtimeNetWrite(handle, 0, message1, 6);
	  printf("offset[0] write msg ok.[%s]\n", message1);

	  /* Polling sleep. */
	  Sleep(3000);
	  char  message2[10+1];
	  memset(message2, 0, sizeof(message2));
	  sprintf(message2, "nihao%05d", i++);
	  RealtimeNetWrite(handle, 100, message2, 10);
	  printf("offset[100] write msg ok.[%s]\n", message2);

	  /* Polling sleep. */
	  Sleep(3000);
	  char *message3 = "this is dds test";
	  RealtimeNetWrite(handle, 256, message3, 16);
	  printf("offset[256] write msg ok.[%s]\n", message3);
	  /* Polling sleep. */
	  Sleep(3000);
  }
  

  RealtimeNetFree(handle);
  return EXIT_SUCCESS;
}
