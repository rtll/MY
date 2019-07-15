#ifndef __SOCKET_H__
#define __SOCKET_H__

class Socket
{
public:
	Socket();
	explicit //说明符，使该类无法被复制初始化，只能直接构造
	Socket(int fd);

	int fd() const ;

	void shutdownWrite();

	~Socket();

private:
	int _fd;
};

#endif
