#include <stdio.h>
#include <stdlib.h>

#include "dds/dds.h"
#include "dds/ddsi/ddsi_xt_typeinfo.h"
#include "dds/ddsi/ddsi_config.h"
#include "dds/ddsrt/hopscotch.h"

#include "HelloWorldData.h"
#include "RealtimeNetMiddleware.h"
#include "RealtimeNetMiddleware_inner.h"
#include "RealtimeNetMsg.h"

#define MAX_SAMPLES 1

static uint32_t typecache_hash (const void *vinfo)
{
  const TopicMapInnerItem *info = vinfo;

 return (uint32_t) (((info->offset + UINT64_C (16292676669999574021)) * UINT64_C (10242350189706880077)) >> 32);   
}

static int typecache_equal (const void *va, const void *vb)
{
  const TopicMapInnerItem *a = va;
  const TopicMapInnerItem *b = vb;
  return a->offset == b->offset;
}


void RealtimeNetSetParam(RtNetInitParam *initParam, RtNetHandle * handle)
{
	handle->ddsDominId = initParam->ddsDominId;

	size_t length = strlen(initParam->networkSegment);
	handle->networkSegment = malloc(length + 1);
	memcpy(handle->networkSegment, initParam->networkSegment, length);
	handle->networkSegment[length] = '\0';

	handle->topicMapListLength = initParam->topicMapListLength;
	TopicMapInnerItem *desList = malloc(sizeof(TopicMapInnerItem) * handle->topicMapListLength);
	TopicMapItem *srcList = initParam->topicMapList;
	for (int i = 0; i < handle->topicMapListLength; i++) {
		length = strlen(srcList[i].topicName);
		desList[i].topicName = malloc(length + 1);
		memset(desList[i].topicName, 0, length + 1);
		memcpy(desList[i].topicName, srcList[i].topicName, length);
		//strncpy(desList[i].topicName, srcList[i].topicName, length);

		desList[i].offset = srcList[i].offset;
		desList[i].size = srcList[i].size;
		desList[i].status = srcList[i].status;
		desList[i].buf = NULL;
		if (TOPIC_TYPE_READ == desList[i].status) {
			desList[i].buf = malloc(desList[i].size + 1);
			memset(desList[i].buf, 0, desList[i].size + 1);
		}
		desList[i].num = i;
	}
	handle->topicMapList = desList;
	handle->onReceiverTopic = initParam->onReceiverTopic;
	handle->timeout = initParam->timeout;
	if (0 == handle->timeout) {
		handle->timeout = 3600; //不设置，按照一小时等待
	}

	handle->exitFlg = false;
	ddsrt_mutex_init(&handle->lock);
}


int RealtimeNetInitHash(RtNetHandle *handle)
{
	handle->typecache = ddsrt_hh_new (32, typecache_hash, typecache_equal);
	for (int i = 0; i < handle->topicMapListLength; i++) {
		ddsrt_hh_add (handle->typecache, &handle->topicMapList[i]);
	}
	return 1;
}

int RealtimeNetCreatePart(RtNetHandle *handle)
{
	static struct ddsi_config raw;
	ddsi_config_init_default(&raw);
	raw.depr_networkAddressString = handle->networkSegment;
	dds_create_domain_with_rawconfig(handle->ddsDominId, &raw);
	
	handle->participant = dds_create_participant(handle->ddsDominId, NULL, NULL);
	if (handle->participant < 0) {
		DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-handle->participant));
		return 0;
	}
	
	return 1;
}

int RealtimeNetCreateTopicAndOper(RtNetHandle *handle)
{
	printf("===   call  RealtimeNetCreateTopicAndOper\n");
	int ret = 0;

	/* Create a reliable Reader. */
	dds_qos_t *qos;
	qos = dds_create_qos();
	dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(handle->timeout));

	for (int i = 0; i < handle->topicMapListLength; i++) {
		/* Create a Topic. */
		dds_entity_t * topic = &(handle->topicMapList[i].topic);
		char * topicName = handle->topicMapList[i].topicName;
		*topic = dds_create_topic(
			handle->participant, &RealtimeNetData_Msg_desc, topicName, NULL, NULL);
		if (*topic < 0)
		{
			DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-*topic));
			ret = 0;
			goto error;
		}

		if (TOPIC_TYPE_WRITE == handle->topicMapList[i].status) {
			/* Create a Writer. */
			dds_entity_t * writer = &(handle->topicMapList[i].operator);
			*writer = dds_create_writer(handle->participant, *topic, NULL, NULL);
			if (*writer < 0) {
				DDS_FATAL("dds_create_writer: %s\n", dds_strretcode(-*writer));
				ret = 0;
				goto error;
			}
		}
		else {
			/* Create a reader. */
			dds_entity_t * reader = &(handle->topicMapList[i].operator);
			*reader = dds_create_reader(handle->participant, *topic, qos, NULL);
			if (*reader < 0) {
				DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-*reader));
				ret = 0;
				goto error;
			}
			else {
				printf("create reader ok.reader:%d\n", *reader);
			}
		}
	}

	ret = 1;

error:
	dds_delete_qos(qos);
	return ret;
}

int RealtimeNetCreateCond(RtNetHandle *handle)
{
	//dds_entity_t waitset;
	dds_entity_t reader;
	handle->waitset = dds_create_waitset(handle->participant);
	//所有reader注册条件触发
	for (int i = 0; i < handle->topicMapListLength; i++) {
		if(TOPIC_TYPE_READ == handle->topicMapList[i].status) {
			reader = handle->topicMapList[i].operator;
			dds_entity_t readcond = dds_create_readcondition(reader, DDS_ANY_STATE);
			dds_waitset_attach(handle->waitset, readcond, reader);
		}
	}

	return 1;
}

int RealtimeNetMsgThread(void * inPara)
{
	RtNetHandle *handle = inPara;
	printf("new thread ok\n");

	int ret;
	dds_attach_t triggered_reader;
	while (!handle->exitFlg) {
		//wl_add  相对时间暂时定位1秒
		dds_time_t tstop = dds_time() + DDS_SECS(1);
		triggered_reader = 0;
		ret = dds_waitset_wait_until(handle->waitset, &triggered_reader, 1, tstop);
		if (ret < 0) {
			printf("dds_waitset_wait_until ret err: %d\n", ret);
			continue;
		}
		else if (0 == ret) {
			printf("dds_waitset_wait_until timeout . do nothing\n");
			continue;
		}
		else {
			printf("dds_waitset_wait_until ok。 ret : %d\n", ret);
		}

		void *samples[MAX_SAMPLES] = { NULL };
		dds_sample_info_t infos[MAX_SAMPLES];
		ret = dds_take((dds_entity_t)triggered_reader, samples, infos, 1, 1);
		if (ret <= 0)
		{
			printf("dds_take ret <0: %d。 continue run\n", ret);
			continue;
		}

		//printf("dds_take return ok: %d\n", ret);
		if (infos[0].valid_data) {
			printf("valid_data is true\n");
			RealtimeNetData_Msg *msg = samples[0];
			if (NULL != msg) {

				int num = RealtimeNetFindTopic(handle, msg->offset);
				if (num < 0) {
					printf("offset[%d] not found topic. read error.\n", msg->offset);
					ret = dds_return_loan((dds_entity_t)triggered_reader, samples, 1);
					return 0;
				}
				TopicMapInnerItem *item = &handle->topicMapList[num];
				if (item->size != msg->data._length) {
					printf("recv msg length is err. length[%d], topic size[%d]\n", msg->data._length, item->size);
					continue;
				}
				ddsrt_mutex_lock(&handle->lock);
				memcpy(item->buf, msg->data._buffer, item->size);
				ddsrt_mutex_unlock(&handle->lock);
				printf("dds_take:Message (%d, %s)\n", msg->offset, item->buf);
				if (handle->onReceiverTopic) {
					handle->onReceiverTopic(item->topicName, msg->offset, item->buf, item->size);
				}
				ret = dds_return_loan((dds_entity_t)triggered_reader, samples, 1);
			}
		}
		else {
			printf("valid_data is false\n");
		}

	}

	printf("new thread finish\n");
	fflush(stdout);
	return 1;
}

int RealtimeNetCreateThread(RtNetHandle *handle)
{
	
	bool haveReader = false;

	//所有reader注册条件触发
	for (int i = 0; i < handle->topicMapListLength; i++) {
		if (TOPIC_TYPE_READ == handle->topicMapList[i].status) {
			haveReader = true;
			break;
		}
	}

	if (!haveReader)
		return 1;
	
	dds_return_t rc;
	ddsrt_threadattr_t tattr;
	ddsrt_threadattr_init(&tattr);
	rc = ddsrt_thread_create(&(handle->tid), "recv", &tattr, RealtimeNetMsgThread, handle);	
	return 1;
}

void * RealtimeNetInit(RtNetInitParam initParam)
{
	printf("===   call  RealtimeNetInit\n");
	int ret = 0;
	RtNetHandle *handle = malloc(sizeof(RtNetHandle));

	/*inputpara to handle */
	RealtimeNetSetParam(&initParam, handle);

	RealtimeNetInitHash(handle);

	  /* Create a Participant. */
	ret = RealtimeNetCreatePart(handle);
	if (0 == ret) {
		return NULL;
	}
	
	/* create topic and operator */
	ret = RealtimeNetCreateTopicAndOper(handle);
	if (0 == ret)	{
		return NULL;
	}
	   
	/* create cond */
	ret = RealtimeNetCreateCond(handle);
	if (0 == ret) {
		return NULL;
	}

	//create thread read msg
	RealtimeNetCreateThread(handle);

	return handle;
}





int	RealtimeNetRead(void* pContext, unsigned int offset, void* pBuffer, unsigned int size)
{
	printf("===   call  RealtimeNetRead\n");
	RtNetHandle *handle;
	handle = pContext;

	int num = RealtimeNetFindTopic(handle, offset);
	if (num < 0) {
		printf("offset[%d] not found topic. read error.\n", offset);
		return 0;
	}

	if (size != handle->topicMapList[num].size) {
		printf("size is error. inputsize[%d], topicsize[%d]\n", size, handle->topicMapList[num].size);
		return 0;
	}
	ddsrt_mutex_lock(&handle->lock);
	memcpy(pBuffer, handle->topicMapList[num].buf, size);
	ddsrt_mutex_unlock(&handle->lock);

	return 1;
}

int RealtimeNetFindTopic(RtNetHandle *handle, unsigned int offset)
{
	TopicMapInnerItem inItem;
	inItem.offset = offset;
	TopicMapInnerItem * pOutItem;

	pOutItem = ddsrt_hh_lookup (handle->typecache, &inItem);
	if (NULL == pOutItem) {
		return -1;
	}
	printf("lookup result. offset[%d], topicname[%s], num[%d]\n", offset, pOutItem->topicName, pOutItem->num);

	return pOutItem->num;

}

int		RealtimeNetWrite(void* pContext, unsigned int offset, void* pBuffer, unsigned int size)
{
	printf("===   call  RealtimeNetWrite\n");
	RtNetHandle *handle;
	RealtimeNetData_Msg msg;
	handle = pContext;

	int num = RealtimeNetFindTopic(handle, offset);
	if (num < 0) {
		printf("offset[%d] not found topic. write error.\n", offset);
		return 0;
	}

	if (size != handle->topicMapList[num].size) {
		printf("input size is error. input size[%d], topic size[%d]\n", size, handle->topicMapList[num].size);
		return 0;

	}

	/* Create a message to write. */
	msg.offset = offset;
	msg.data._maximum = size;
	msg.data._length = size;
	msg.data._buffer = pBuffer;
	msg.data._release = false;

	dds_return_t rc;;
	rc = dds_write(handle->topicMapList[num].operator, &msg);
	if (rc != DDS_RETCODE_OK)
		DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));

	return 1;
}

void  RealtimeNetFree(void* pContext)
{
	RtNetHandle *handle;
	handle = pContext;

	handle->exitFlg = true;
	dds_sleepfor(DDS_MSECS(1010));
	
	ddsrt_mutex_destroy(&handle->lock);
	printf("===  call  RealtimeNetFree\n");
	ddsrt_hh_free (handle->typecache);
	dds_delete(handle->participant);

	TopicMapInnerItem *desList = handle->topicMapList;
	for (int i = 0; i < handle->topicMapListLength; i++) {
		free(desList[i].topicName);
		if (TOPIC_TYPE_READ == desList[i].status) {
			free(desList[i].buf);
		}
	}	
	free(handle->topicMapList);	
	free(handle->networkSegment);
	free(handle);
}
