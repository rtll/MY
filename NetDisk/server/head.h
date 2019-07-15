#ifndef __HEAD_H__ //如果没有定义（前后两个下滑线，中间下划线代表.
#define __HEAD_H__ //则定义
#include<sys/uio.h> 
#include<sys/epoll.h>
#include<sys/socket.h>
#include<netdb.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/sem.h>
#include<sys/time.h>
#include<sys/select.h>
#include<sys/msg.h>
#include<sched.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<errno.h>
#include<syslog.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<dirent.h>
#include<time.h>
#include<pwd.h>
#include<grp.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<signal.h>
#include<pthread.h>
#define ARGS_CHECK(argc,val){if(argc!=val){printf("error args\n");return -1;}}
#define ERROR_CHECK(ret,retval,funcName){if(ret==retval)\
	{printf("%d:",__LINE__);fflush(stdout);perror(funcName);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName){if(ret!=0)\
	{printf("%s:%s\n",funcName,strerror(ret));return -1;}}

#define TCP_INIT()({int socketFD;socketFD=socket(AF_INET,SOCK_STREAM,0);\
    ERROR_CHECK(socketFD,-1,"socket");int rett;int reuse=1;\
    rett=setsockopt(socketFD,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));\
    ERROR_CHECK(rett,-1,"setsockopt");\
    struct sockaddr_in ser;bzero(&ser,sizeof(ser));\
    ser.sin_family=AF_INET;\
    ser.sin_addr.s_addr=inet_addr("192.168.3.198");\
    ser.sin_port=htons(2000);\
    rett=bind(socketFD,(struct sockaddr*)&ser,sizeof(ser));ERROR_CHECK(rett,-1,"bind");\
    listen(socketFD,10);\
    socketFD;})
#endif
