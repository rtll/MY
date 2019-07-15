#ifndef __INETADDRESS_H__
#define __INETADDRESS_H__

#include <arpa/inet.h>
#include <string>
using std::string;

class InetAddress
{
public:
	explicit
	InetAddress(unsigned short port);
	InetAddress(const string & ip, unsigned short port);
	InetAddress(const struct sockaddr_in & addr);

	string ip() const;
	unsigned short port() const;
	struct sockaddr_in * getInetAddressPtr() {	return &_addr;	}
	
private:
	struct sockaddr_in _addr;
};

#endif
