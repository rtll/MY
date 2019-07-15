#ifndef __FACTORY_H__
#define __FACTORY_H__
#include"head.h"
#include"work_que.h"
#include"tranfile.h"

typedef struct {
    pthread_t *pthid;//存储线程ID的起始地址
    int threadNUM;//要创建的线程数目
    pthread_cond_t cond;//每个线程都要使用的条件变量
    que_t que;//生产者，消费者操作的队列
    short startFlag;//工厂启动标志
}factory_t,*pfactory_t;

void factoryInit(pfactory_t,int,int);
void factoryStart(pfactory_t);
int sign_in(int);
int log_in(int,char*);
int user_ctr(int,char*);
//#define FILENAME "file"

#endif
