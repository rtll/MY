#include "SocketIO.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

SocketIO::SocketIO(int fd)
: _fd(fd)
{}

//一次读取长度为n个字节的数据
int SocketIO::readn(char * buff, int len)
{
	int left = len;
	char * p = buff;
	while(left > 0) {
		int ret = ::read(_fd, p, left);
		//返回-1且errno为中断信号EINTR，不管它
        if(ret == -1 && errno == EINTR)
			continue;
        //返回-1且errno非中断信号，为read错误
		else if(ret == -1) {
			perror("read");
			return len - left;
		}
        //返回0表示客户端断开
		else if(ret == 0) {
			return len - left;
		} 
        else {
			left -= ret;
			p += ret;
		}
	}
	return len - left;
}

//每一次读取一行数据
int SocketIO::readline(char * buff, int maxlen)
{
	int left = maxlen - 1;
	char * p = buff; 
	int ret;
	int total = 0;
	while(left > 0) {
        //不移走内核缓冲区中数据的方式获取数据来看看
		ret = recvPeek(p, left);		
		//查找 '\n'
		for(int idx = 0; idx != ret; ++idx) {
            //正好该行有换行
            //则可以在内核缓冲区中读取换行前的部分并返回
            //而此时该部分也从缓冲区中被移走
			if(p[idx] == '\n') {
				int sz = idx + 1;
				readn(p, sz);
				total += sz;
				p += sz;
				*p = '\0';
				return total;
			}
		}
		//如果没有发现 '\n'
        //则可以获取并移走内核态缓冲区中该ret长度的数据
		readn(p, ret);
		left -= ret;
		p += ret;
		total += ret;
	}
	*p = '\0';// 最终没有发现'\n'
	return total;
}

//MSG_PEEK只从内核态获取数据到用户态，不把数据从内核态删除
int SocketIO::recvPeek(char * buff, int len)
{
	int ret;
	do {
		ret = ::recv(_fd, buff, len, MSG_PEEK);
	} while(ret == -1 && errno == EINTR);
	return ret;
}

	
int SocketIO::writen(const char * buff, int len)
{
	int left = len;
	const char * p = buff;
	while(left > 0) {
		int ret = ::write(_fd, p, left);
		//返回-1且errno为中断信号EINTR，不管他
        if(ret == -1 && errno == EINTR)
			continue;
        //返回-1且errno非中断信号，write错误
		else if(ret == -1) {
			perror("write");
			return len - left;
		}
        //write只有两种情况，这里是ret>=0，正常写入
        else {
			left -= ret;
			p += ret;
		}
	}
	return len - left;
}
