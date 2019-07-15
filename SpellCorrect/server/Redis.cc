#include"Redis.h"

bool Redis::connect()
{
    _connect = redisConnect(_host.c_str(),_port);
    if(_connect->err){
        redisFree(_connect);
        printf(" - REDIS - Connect to redisServer faile\n");
        printf("Error : %s\n",_connect->errstr);
        return 0;
    }
    printf(" - REDIS - Connect to redisServer Success\n");
    setLRU();
	return 1;
}

void Redis::setLRU()
{
    _reply = (redisReply*)redisCommand(_connect,"config set maxmemory-policy allkeys-lru");
    if( NULL == _reply )     return;
    if( !(_reply->type == REDIS_REPLY_STATUS && strcasecmp(_reply->str,"OK")==0) ){
        freeReplyObject(_reply);  return;
    }
    freeReplyObject(_reply);

    _reply = (redisReply*)redisCommand(_connect,"config set maxmemory %s",_cache.c_str());
    if( NULL == _reply )   return;
    if( !(_reply->type == REDIS_REPLY_STATUS && strcasecmp(_reply->str,"OK")==0) ){
        freeReplyObject(_reply);  return;
    }
    
    cout<<" - REDIS - Set LRU mode with "<<_cache<<" cache capacity Success"<<endl;
    freeReplyObject(_reply);
}

void Redis::sett(string key, string value)
{
    _reply = (redisReply*)redisCommand(_connect,"SET %s %s", key.c_str(), value.c_str());
    if( NULL == _reply ){
        cout<<" - REDIS - Execut SET "<<key<<" "<<value<<" failure"<<endl;
        return;
    }
    if( !(_reply->type == REDIS_REPLY_STATUS && strcasecmp(_reply->str,"OK")==0) ){
        cout<<" - REDIS - Failed to execute command [SET "<<key<<" "
            <<value<<"]:"<<_reply->str<<endl;
        freeReplyObject(_reply);  return;
    }
    cout<<" - REDIS - Succeed to execute command [SET "<<key<<"]"<<endl;
    freeReplyObject(_reply);
    //插入键值对后手动将缓存同步到磁盘rdb文件
    save();
}

string Redis::get(string key)
{
	_reply = (redisReply*)redisCommand(_connect,"GET %s",key.c_str());
    if (_reply->type != REDIS_REPLY_STRING ){
        cout<<" - REDIS - Failed to execute command [GET "<<key<<"]"<<endl;
        freeReplyObject(_reply); return "";
    }
    string str = _reply->str;
    cout<<" - REDIS - Succeed to execute command [GET "<<key<<"]"<<endl;
	freeReplyObject(_reply);
    return str;
}

bool Redis::save()
{
    _reply = (redisReply*)redisCommand(_connect,"save");
    if( NULL == _reply ){
        cout<<" - REDIS - Execut SAVE failure"<<endl;
        return 0;
    }
    if( !(_reply->type == REDIS_REPLY_STATUS && strcasecmp(_reply->str,"OK")==0) ){
        cout<<" - REDIS - Failed to execute command [SAVE]:"
            <<_reply->str<<endl;
        freeReplyObject(_reply); return 0;
    }
    cout<<" - REDIS - Succeed to execute command [SAVE]"<<endl;
    freeReplyObject(_reply); return 1;
}

Redis::Redis(string host, int port,string cache)
:_host(host)
,_port(port)
,_cache(cache)
{}

Redis::~Redis()
{
    //即使有默认的rdb快照配置，保险起见退出时手动存一次缓存到硬盘
    if(save())
        cout<<" - REDIS - Save cache into file success!"<<endl;
    else
        cout<<" - REDIS - Save cacheinto file failure."<<endl;
	if(!_reply) freeReplyObject(_reply);
    if(!_connect) redisFree(_connect) ;
}
