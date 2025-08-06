#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "RealtimeNetMiddleware.h"
#include <windows.h>
#include <QThread>
#include <QString>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    test();
}


extern "C" {
void recvMsg(char* topicName, unsigned int offset, void* pBuffer, unsigned int size)
{
    char buf[256];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, pBuffer, size);
    printf("callback recvMsg: topicName[%s], offset[%d], pBuffer[%s], size[%d]\n", topicName, offset, (char *)buf, size);
    qDebug() << "callback recvMsg: topicName: " << topicName  << " pBuffer: " << buf;
    fflush (stdout);
}
}

void MainWindow::test()
{
    TopicMapItem  topicMapList[3];
    memset(&topicMapList, 0, sizeof(topicMapList));
    char buf1[] = "dds_msg1";
    topicMapList[0].topicName = buf1;
    topicMapList[0].offset = 0;
    topicMapList[0].size = 6;
    topicMapList[0].status = TOPIC_TYPE_READ;
    char buf2[] = "dds_msg2";
    topicMapList[1].topicName = buf2;
    topicMapList[1].offset = 100;
    topicMapList[1].size = 10;
    topicMapList[1].status = TOPIC_TYPE_READ;
    char buf3[] = "dds_msg3";
    topicMapList[2].topicName = buf3;
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
    Sleep(5000);


    char buf[33];
    memset(buf, 0, sizeof(buf));
    RealtimeNetRead(handle, 0, buf, 6);
    printf("readmsg offset[0]'s msg is :%s\n", buf);
    qDebug() << "readmsg. msg is: " << buf;



    RealtimeNetRead(handle, 100, buf, 10);
    printf("readmsg offset[100]'s msg is :%s\n", buf);
    qDebug() << "readmsg. msg is: " << buf;

    RealtimeNetRead(handle, 256, buf, 16);
    printf("readmsg offset[256]'s msg is :%s\n", buf);
    qDebug() << "readmsg. msg is: " << buf;
    fflush (stdout);




}


MainWindow::~MainWindow()
{
    delete ui;
}

