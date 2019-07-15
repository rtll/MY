#ifndef __REDIS_H__
#define __REDIS_H__

#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <hiredis/hiredis.h>
using namespace std;

class Redis
{
public:
    static void destroy(){
        if(_pRedis)
            delete _pRedis;
    }

    static Redis* getInstance(){
        if(!_pRedis){
            _pRedis = new Redis();
            atexit(destroy);
        }
        return _pRedis;
    }
	
    bool connect();
    void setLRU();
    void sett(string key, string value);
    string get(string key);

private:
    //使用默认实参时只需在声明时赋值，且实参名可不写
    //而在定义时不能写实参
    Redis(string = "127.0.0.1",int = 6379,string = "1mb");
    ~Redis();
    bool save();
    string _host;
    int _port;
    string _cache;
    redisContext* _connect;
	redisReply* _reply;
    static Redis* _pRedis;
};

#endif
