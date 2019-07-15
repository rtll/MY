#include"work_que.h"

void queInit(pque_t pq,int capacity)
{
    memset(pq,0,sizeof(que_t));
    pq->qCapacity=capacity;
    pthread_mutex_init(&pq->mutex,NULL);
}

void queInsert(pque_t pq,pnode_t pn)
{
    if(NULL==pq->qtail){
        pq->qhead=pn;
        pq->qtail=pn;
    } 
    else{
      pq->qtail->pnext=pn;
      pq->qtail=pn;
    }
    pq->qSize++;
}

int queGet(pque_t pq,pnode_t* ppcurr)
{   
    if(0==pq->qSize){
        return -1;
    }
    else{ 
        *ppcurr=(pq->qhead);
        pq->qhead=pq->qhead->pnext;
        if(NULL==pq->qhead){
           pq->qtail=NULL;
        }
    pq->qSize--;
    return 0;
    }
}
