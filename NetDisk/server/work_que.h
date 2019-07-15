#ifndef __WORK_QUE__
#define __WORK_QUE__

#include"head.h"
typedef struct node{
    int newfd;
    struct node *pnext;
}node_t,*pnode_t;

typedef struct {
    pnode_t qhead,qtail;
    int qCapacity;//队列容量
    int qSize;//队列元素实时个数
    pthread_mutex_t mutex;//线程互斥锁
}que_t,*pque_t;
void queInit(pque_t,int);
void queInsert(pque_t,pnode_t);
int queGet(pque_t,pnode_t*);
#endif
