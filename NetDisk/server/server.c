#include "factory.h"

int main(int argc,char* argv[])
{
    ARGS_CHECK(argc,3);
    factory_t f;
    int threadNum=atoi(argv[1]);
    int capacity=atoi(argv[2]);
    factoryInit(&f,threadNum,capacity);
    factoryStart(&f);
    int sfd=TCP_INIT();
    int newfd;
    pque_t pq=&f.que;//定义一个队列指针
    while(1){
        newfd=accept(sfd,NULL,NULL);//接受客户端连接
        printf("New client connected complete,sfd=%d\n",sfd);
        pnode_t pnew=(pnode_t)calloc(1,sizeof(node_t));
        pnew->newfd=newfd;
        pthread_mutex_lock(&pq->mutex);
        queInsert(pq,pnew);//将客户端插入工厂队列
        pthread_mutex_unlock(&pq->mutex);
        pthread_cond_signal(&f.cond);//接到满足条件变量的信号，唤醒子线程
    }
    return 0;
}

