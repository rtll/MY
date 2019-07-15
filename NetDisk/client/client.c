#include"head.h"

int main()
{
	int sfd=TCP_INIT();
    int ret;
    char name[20]={0};
    Train_t loginFlag;
    while(1){
        bzero(&loginFlag,sizeof(loginFlag));
        system("clear");
        printf("Connect server complete,sfd=%d\n",sfd);
        printf("注册请输入1，登录请输入2：");
        loginFlag.dataLen=getchar();
        if(loginFlag.dataLen==50){//登录
            ret=sendCycle(sfd,(char*)&loginFlag,4+loginFlag.dataLen);//发送登录标记
            ERROR_CHECK(ret,-1,"sendloginflag");
            ret=log_in(sfd,name);
            ERROR_CHECK(ret,-1,"log_in");
            if(1==ret)continue;
            ret=user_ctr(sfd,name);//用户操作
            ERROR_CHECK(ret,-1,"log_in");
        }
        else if(loginFlag.dataLen==49){//注册
            ret=sendCycle(sfd,(char*)&loginFlag,4+loginFlag.dataLen);//发送注册标记
            ERROR_CHECK(ret,-1,"sendloginflag");
            ret=sign_in(sfd);
            ERROR_CHECK(ret,-1,"sigh_in");
            if(1==ret)continue;
        }
        else{
            printf("输入有误,3秒后请重新输入\n");
        }
    }
    close(sfd);
	return 0;
}

