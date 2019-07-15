#include "InetAddress.h"
#include <string.h>

InetAddress::InetAddress(unsigned short port)
{
    memset(&_addr, 0, sizeof(struct sockaddr_in));
	_addr.sin_family = AF_INET;  //地址族 = IPv4协议
	_addr.sin_port = htons(port);//主机字节序转成short网络字节序列
	_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("0.0.0.0") 代表本机地址(一个服务器可能有多个网卡)
}

InetAddress::InetAddress(const string & ip, unsigned short port)
{
	memset(&_addr, 0, sizeof(struct sockaddr_in));
	_addr.sin_family = AF_INET;  //小端模式      大端模式
	_addr.sin_port = htons(port);//主机字节序转成short网络字节序列
	_addr.sin_addr.s_addr = inet_addr(ip.c_str());//将点分十进制数值转换为网络字节序的32位二进制数值
}

InetAddress::InetAddress(const struct sockaddr_in & addr)
: _addr(addr)
{
}

string InetAddress::ip() const
{
	return string(inet_ntoa(_addr.sin_addr));//将网络字节序的32位二进制IP（结构体）转为点分十进制字符串
}

unsigned short InetAddress::port() const
{
	return ntohs(_addr.sin_port);
}
