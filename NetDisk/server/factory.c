#include "factory.h"
void* downLoadFile(void *p)//子线程函数，需返回函数指针
{
    //判断队列是否为空，为空就睡觉，不为空就干活
    pfactory_t pf=(pfactory_t)p;
    pque_t pq=&pf->que;
    pnode_t pcurr;
    int successFlag,loginFlag,dataLen,ret;
    char buf[1000]={0};
    char name[20]={0};
    while(1){
        pthread_mutex_lock(&pq->mutex);
        if(0==pq->qSize){
            pthread_cond_wait(&pf->cond,&pq->mutex);//睡觉，等待对应条件变量，释放锁
        }
        successFlag=queGet(pq,&pcurr);//是否获取线程成功，成功返回0
        pthread_mutex_unlock(&pq->mutex);
        if(!successFlag){//获取成功才与客户端交互
            while(1){
                bzero(buf,sizeof(buf));
                //接收登录或注册标记
                recvCycle(pcurr->newfd,(char*)&dataLen,4);
                recvCycle(pcurr->newfd,buf,dataLen);
                loginFlag=dataLen-49;
                printf("loginFlag = %d\n",dataLen);
                if(loginFlag){//登录
                    ret=log_in(pcurr->newfd,name);
                    if(ret){
                        printf("登录失败\n");
                        continue;
                    }    
                    user_ctr(pcurr->newfd,name);
                }
                else{//注册,只有登录后才能跳出这个循环
                    ret=sign_in(pcurr->newfd);
                    if(0==ret){
                        printf("注册成功\n");
                    }
                    else{
                        printf("注册失败\n");
                    }
                }
            }
        }
    }
    free(pcurr);            
}

void factoryInit(pfactory_t pf,int threadNum,int capacity)
{
    memset(pf,0,sizeof(factory_t));//为了可复用性
    pf->threadNUM=threadNum;
    pf->pthid=(pthread_t*)calloc(threadNum,sizeof(pthread_t));
    pthread_cond_init(&pf->cond,NULL);
    queInit(&pf->que,capacity) ;   
}

void factoryStart(pfactory_t pf)
{
    if(0==pf->startFlag){
        for(int i=0;i<pf->threadNUM;i++){//循环创建线程
            pthread_create(pf->pthid+i,NULL,downLoadFile,pf);
        }
    } 
}
