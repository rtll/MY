#ifndef __EchoServer_H__ 
#define __EchoServer_H__
#include "Threadpool.h"
#include "TcpServer.h"
#include "Mission.h"
#include<iostream>
using namespace std;

class EchoServer
{
public:
    EchoServer(size_t threadNum,size_t queSize
               ,string ip,unsigned short port);

private:
	static void onConnection(const TcpConnectionPtr & conn);
    static void onMessage(const TcpConnectionPtr & conn);
	static void onClose(const TcpConnectionPtr & conn);
	
    Threadpool _threadpool;
	static Threadpool* _pthreadpool; 
    TcpServer _server;
};

#endif
