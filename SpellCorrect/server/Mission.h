#ifndef __Mission_H__ 
#define __Mission_H__
#include"TcpConnection.h"
#include"Dictionary.h"
#include"Index.h"
#include"Redis.h"
#include<queue>
using namespace std;

struct Result
{
    int distanc;
    string candidate_word;
    int frequency;
};

//NB操作
struct cmp{
    bool operator()(const Result & lhs,const Result & rhs){
        if(lhs.distanc == rhs.distanc)
            return lhs.frequency < rhs.frequency;
        return lhs.distanc > rhs.distanc;
    }
};

class Mission
{
public:
	Mission(const string & msg,const TcpConnectionPtr & conn)
	:_msg(msg)
	,_conn(conn)
    ,_pDict(Dictionary::getInstanceDict())
    ,_pIndex(Index::getInstanceIndex())
    ,_pRedis(Redis::getInstance())
    {}

	//运行在线程池的某一个子线程中
	void process();

private:
    inline string cleanup_str(const string &);
    inline string getEnResult(const string &);
    inline string getChResult(const string &);
    inline int editDistance(const string &,const string &);
    string _msg;
	TcpConnectionPtr _conn;
    Dictionary* _pDict;
    Index* _pIndex;
    Redis* _pRedis;
    priority_queue<Result,vector<Result>,cmp> _que;
    int _K = 5;
};

#endif
