#include "TcpConnection.h"
#include "InetAddress.h"
#include "EventLoop.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <sstream>

TcpConnection::TcpConnection(int fd, EventLoop * loop)
: _sock(fd)
, _socketIo(fd)
, _localAddr(getLocalAddr())
, _peerAddr(getPeerAddr())
, _isShutdwonWrite(false)
, _loop(loop)
{
}

TcpConnection::~TcpConnection()
{
    //以防连接没有断开，保险措施
	if(!_isShutdwonWrite) {
		shutdown();
	}
}

string TcpConnection::receive()
{
	char buff[65536] = {0};
	_socketIo.readline(buff, sizeof(buff));
	return string(buff);
}
	
void TcpConnection::send(const string & msg)
{
	_socketIo.writen(msg.c_str(), msg.size());
}

void TcpConnection::sendInLoop(const string & msg)
{
	_loop->runInLoop(std::bind(&TcpConnection::send, this, msg));
}

void TcpConnection::shutdown()
{
	if(!_isShutdwonWrite) {
		_isShutdwonWrite = true;
		_sock.shutdownWrite();
	}
}

string TcpConnection::toString() const
{
    //字符串输出流
	std::ostringstream oss;
	oss << _localAddr.ip() << ":" << _localAddr.port() << " --> "
		<< _peerAddr.ip() << ":" << _peerAddr.port();
	return oss.str();
}


InetAddress TcpConnection::getLocalAddr()
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr);
	//getsockname()是系统函数
    //第二个参数接收本地地址信息结构体
    //无错误返回0，否则返回
    if(getsockname(_sock.fd(), (struct sockaddr*)&addr, &len) == -1) {
		perror("getsockname");
	}
    //返回封装好的本地网络地址信息
	return InetAddress(addr);
}

InetAddress TcpConnection::getPeerAddr()
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr);
	//获取fd对端（客户端）的地址信息，其他同getsockname()
    if(getpeername(_sock.fd(), (struct sockaddr*)&addr, &len) == -1) {
		perror("getpeername");
	}
	return InetAddress(addr);
}

void TcpConnection::setConnectionCallback(const TcpConnectionCallback & cb)
{
	_onConnection = std::move(cb);
}

void TcpConnection::setMessageCallback(const TcpConnectionCallback & cb)
{
	_onMessage = std::move(cb);
}

void TcpConnection::setCloseCallback(const TcpConnectionCallback & cb)
{
	_onClose = std::move(cb);
}

void TcpConnection::handleConnectionCallback()
{
	if(_onConnection) {
		_onConnection(shared_from_this());
	}
}

void TcpConnection::handleMessageCallback()
{
	if(_onMessage) {
		_onMessage(shared_from_this());
	}
}

void TcpConnection::handleCloseCallback()
{
	if(_onClose) {
		_onClose(shared_from_this());
	}
}
