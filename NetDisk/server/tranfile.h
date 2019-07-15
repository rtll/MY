#ifndef __TRANFILE_H
#define __TRANFILE_H
#define _GNU_SOURCE
#include "head.h"
//小火车
typedef struct{
	int dataLen;//控制数据
	char buf[1000];//传送内容
}Train_t;

int sendCycle(int,char*,int);
int recvCycle(int,char*,int);
int tranFile(int,char*);
int getFile(int);
#endif
