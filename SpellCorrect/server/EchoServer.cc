#include"EchoServer.h"

EchoServer::EchoServer(size_t threadNum,size_t queSize
           ,string ip,unsigned short port)
:_threadpool(threadNum,queSize),_server(ip,port)
{
    _threadpool.start();
    _pthreadpool = &_threadpool;
    _server.setConnectionCallback(onConnection);
    _server.setMessageCallback(onMessage);
    _server.setCloseCallback(onClose);
    _server.start();
}

void EchoServer::onConnection(const TcpConnectionPtr & conn)
{
    cout << conn->toString() << " has connected!" << endl;
    conn->send("welcome to server.");
}

void EchoServer::onMessage(const TcpConnectionPtr & conn)
{
    //收到客户端的信息
    cout << "onMessage...." << endl;
    string msg = conn->receive();
    cout << ">> receive msg from client: " << msg << endl;
 
    //建立新任务
    Mission mission(msg, conn);
    //并让线程池添加该任务去处理，本质是Mission::process()中处理
    _pthreadpool->addTask(std::bind(&Mission::process,mission));
}

void EchoServer::onClose(const TcpConnectionPtr & conn)
{
    cout << "onClose...." << endl;
    cout << conn->toString() << " has closed!" << endl;
}

