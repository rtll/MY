#include "EventLoop.h" 
#include "Acceptor.h"
#include "TcpConnection.h"

#include <unistd.h>
#include <assert.h>
#include <sys/eventfd.h>

#include <iostream>
using std::cout;
using std::endl;

EventLoop::EventLoop(Acceptor & acceptor)
: _efd(createEpollFd())
, _eventfd(createEventFd())
, _acceptor(acceptor)
, _eventList(1024)//vector这里初始化需设初值
, _isLooping(false)
{
    //开始监听客户端接入或发来消息
	addEpollFdRead(_acceptor.fd());
    //开始监听子线程发来处理后的数据
	addEpollFdRead(_eventfd);
}

void EventLoop::loop()
{
	_isLooping = true;
	while(_isLooping) {
		waitEpollFd();
	}
}

void EventLoop::unloop()
{
	if(_isLooping) 
		_isLooping = false;
}

void EventLoop::runInLoop(Functor && cb)
{
	{
	MutexLockGuard autolock(_mutex);
	_pendingFunctors.push_back(std::move(cb));
	}

	wakeup();
}

void EventLoop::waitEpollFd()
{
	int nready;
	do {
		nready = epoll_wait(_efd, &*_eventList.begin(), _eventList.size(), 5000);
	}while(nready == -1 && errno == EINTR);

	if(nready == -1) {
		perror("epoll_wait");
		return;
	} 
    //超时，5s内没有文件描述符
    else if(nready == 0) {
		cout << ">> epoll_wait timeout!" << endl;
	} else {
		if(nready == (int)_eventList.size()) {
            //vector扩容操作
			_eventList.resize(2 * nready);
		}

		for(int idx = 0; idx != nready; ++idx) {
			int fd = _eventList[idx].data.fd;
			//处理新连接
			if(fd == _acceptor.fd()) {
				if(_eventList[idx].events & EPOLLIN) {
					handleNewConnection();
				}
			}
            //被子线程（计算线程）wakeup,接收处理后的数据
            else if(fd == _eventfd) {
				if(_eventList[idx].events & EPOLLIN) {
					handleRead();
                    //在这里发送数据给客户端
					doPendingFunctors();
				}
			} 
			//去收客户端发来的需要处理的数据
            else {
				if(_eventList[idx].events & EPOLLIN) {
					handleMessage(fd);
				}
			}
		}
	}
}

void EventLoop::handleNewConnection()
{
	int peerfd = _acceptor.accept();
	addEpollFdRead(peerfd);
	TcpConnectionPtr conn(new TcpConnection(peerfd, this));
	//在tcpConnection中设置回调函数
    conn->setConnectionCallback(_onConnection);
	conn->setMessageCallback(_onMessage);
	conn->setCloseCallback(_onClose);

	_conns.insert(std::make_pair(peerfd, conn));

	conn->handleConnectionCallback();
}

void EventLoop::handleMessage(int fd)
{
	bool isClosed = isConnectionClosed(fd);
	auto iter = _conns.find(fd);
    //运行时断言,满足则继续，不满足则报错
	assert(iter != _conns.end());//运行时断言

	if(!isClosed) {
		iter->second->handleMessageCallback();
	}
    //若连接已断开
    else {
		delEpollFdRead(fd);
		iter->second->handleCloseCallback();
		_conns.erase(iter);
	}
}

void EventLoop::handleRead()
{
	uint64_t howmany;
	int ret = ::read(_eventfd, &howmany, sizeof(howmany));
	if(ret != sizeof(howmany)) {
		perror("read");
	}
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
	int ret = ::write(_eventfd, &one, sizeof(one));
	if(ret != sizeof(one)) {
		perror("write");
	}
}

void EventLoop::doPendingFunctors()
{
	vector<Functor> tmp;
	{
		MutexLockGuard autolock(_mutex);
		//tmp有_pendingFunctors中的所有元素了,_pendingFunctors空了
        tmp.swap(_pendingFunctors);
	}

	for(auto & functor : tmp)
	{
		functor();
        //functor来源于在runInLoop()中被push_back入_pendingFunctors的cb
        //runInLoop()在TcpConnection::sendInLoop()中被调用
        //cb的真相是上一步的参数bind(&TcpConnection::send, this, msg)
        //this是把msg用TcpConnection::send()函数处理的TcpConnection对象指针
        //在send()中调用socketIo::writen()把msg发给客户端
	}
}

bool EventLoop::isConnectionClosed(int fd)
{
	int ret;
	do{
		char buff[1024];
        //从缓冲区读取数据，若返回0则对端断开
		ret = recv(fd, buff, sizeof(buff), MSG_PEEK);
	}while(ret == -1 && errno == EINTR);//若中断信号则继续进行

	return (ret == 0);
}
	
int EventLoop::createEpollFd()
{
    //创建一个多路复用的实例，相当于poll_create(0)
	int ret = ::epoll_create1(0);
	if(ret == -1) {
		perror("epoll_create1");
	}
	return ret;
}

int EventLoop::createEventFd()
{
	int ret = ::eventfd(0, 0);
	if(ret == -1) {
		perror("eventfd");
	}
	return ret;
}

void EventLoop::addEpollFdRead(int fd)
{
	struct epoll_event evt;
	evt.data.fd = fd;
	evt.events = EPOLLIN;//读操作
	int ret = epoll_ctl(_efd, EPOLL_CTL_ADD, fd, &evt);
	if(-1 == ret) {
		perror("epoll_ctl");
	}
}

void EventLoop::delEpollFdRead(int fd)
{
	struct epoll_event evt;
	evt.data.fd = fd;
	int ret = epoll_ctl(_efd, EPOLL_CTL_DEL, fd, &evt); 
	if(-1 == ret) {
		perror("epoll_ctl");
	}
}
