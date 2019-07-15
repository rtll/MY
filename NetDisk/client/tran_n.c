#include "head.h"
//循环接收
int recvCycle(int fd,char* p,int totallen)
{
    int recved=0;
    int ret;
    while(recved<totallen){
        ret=recv(fd,p+recved,totallen-recved,0);
        ERROR_CHECK(ret,-1,"recv");
        recved+=ret;
    }
    return 0;
}

int sendCycle(int fd,char* p,int totallen)
{
    int sended=0;
    int ret;
    while(sended<totallen){
        ret=send(fd,p+sended,totallen-sended,0);
        ERROR_CHECK(ret,-1,"recv");
        sended+=ret;
    }
    return 0;
}
