#include "EchoServer.h"
#include "Configure.h"
#include "Dictionary.h"
#include "Index.h"
#include "Redis.h"

Threadpool* EchoServer::_pthreadpool = nullptr ; 
Configure* Configure::_pConfigure = nullptr;
Dictionary* Dictionary::_pDictionay = nullptr; 
Index* Index::_pIndex = nullptr;
Redis* Redis::_pRedis = nullptr;

int main()
{
    Configure::setInstanceConf("../config/config.txt");
    Dictionary::setInstanceDict
        (Configure::getInstanceConf()->enYuliao_path(),
         Configure::getInstanceConf()->enStop_path(),
         Configure::getInstanceConf()->enDict_path(),
         Configure::getInstanceConf()->chYuliao_path(),
         Configure::getInstanceConf()->chStop_path(),
         Configure::getInstanceConf()->chDict_path());
    Index::setInstanceIndex
        (Dictionary::getInstanceDict()->enDict(),
         Configure::getInstanceConf()->enIndex_path(),
         Dictionary::getInstanceDict()->chDict(),
         Configure::getInstanceConf()->chIndex_path());
    Redis::getInstance()->connect();
    
    EchoServer server(4,10,Configure::getInstanceConf()->getIp(),
                      Configure::getInstanceConf()->getPort());
    
	return 0;
}
