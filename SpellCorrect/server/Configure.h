#ifndef __Configure_H__ 
#define __Configure_H__
#include <iostream>
using namespace std;

class Configure
{
public:
    static void destroy(){
        if(_pConfigure)
            delete _pConfigure;
    }

    static Configure* setInstanceConf(const string & pth)
    {
        if(!_pConfigure){
            _pConfigure = new Configure(pth);
        }
        atexit(destroy);
        return _pConfigure;
    }

    static Configure* getInstanceConf(){
        return _pConfigure;
    }
    void readConfig();
    string getIp() const { return _ip; }
    unsigned short getPort() const { return _port; }
    string enYuliao_path() const { return _enYuliao_path; }
    string chYuliao_path() const { return _chYuliao_path; }
    string enStop_path() const { return _enStop_path; }
    string chStop_path() const { return _chStop_path; }
    string enDict_path() const { return _enDict_path; }
    string chDict_path() const { return _chDict_path; }
    string enIndex_path() const { return _enIndex_path; }
    string chIndex_path() const { return _chIndex_path; }

private:
    Configure(const string & pth)
    :_config_path(pth)
    {
        readConfig();
    }
    static Configure* _pConfigure;
    string _config_path;
    string _ip;
    unsigned short _port;
    string _enYuliao_path;
    string _chYuliao_path;
    string _enStop_path;
    string _chStop_path;
    string _enDict_path;
    string _chDict_path;
    string _enIndex_path;
    string _chIndex_path;
};

#endif

