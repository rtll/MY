#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__
#include "Acceptor.h"
#include "EventLoop.h"
#include "TcpConnection.h"

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TcpConnectionCallback = std::function<void(const TcpConnectionPtr&)>;

class TcpServer
{
public:
	TcpServer(const string & ip, unsigned short port);
    void start();

	void setConnectionCallback(TcpConnectionCallback && cb);
	void setMessageCallback(TcpConnectionCallback && cb);
	void setCloseCallback(TcpConnectionCallback && cb);

private: 
	Acceptor _acceptor;
	EventLoop _loop;
};

#endif
