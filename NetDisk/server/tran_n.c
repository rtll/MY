#include "tranfile.h"
//循环发送
int sendCycle(int fd,char* p,int totallen)
{
    int sended=0;
    int ret;
    while(sended<totallen){
        ret=send(fd,p+sended,totallen-sended,0);
        ERROR_CHECK(ret,-1,"send");
        sended+=ret;
    }
    return 0;
}

int recvCycle(int fd,char* p,int totallen)
{
    int recved=0;
    int ret;
    while(recved<totallen){
        ret=recv(fd,p+recved,totallen-recved,0);
        ERROR_CHECK(ret,-1,"send");
        recved+=ret;
    }
    return 0;
}
