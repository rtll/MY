#include"Mission.h"
#include<json/json.h>
#include<algorithm>
#include<cstring>

inline string Mission::cleanup_str(const string & msg)
{
    //客户端传来的msg后缀带有'\n',因此len也比实际多1，需处理
    //一并处理大写转换为小写,去掉可能夹杂的标点
    size_t len = msg.length();
    string res;
    for(size_t i = 0; i != len - 1; ++i){
        if(ispunct(msg[i])||msg[i] == ' ')
            continue;
        else if(msg[i] >= 'A' && msg[i] <= 'Z') 
            res += (msg[i] + 32);    
        else if(msg[i] == '\n')
            break;
        else 
            res += msg[i];
    }
    return res;
}

inline int triple_min(const int & a, const int & b, const int & c)
{
    return a < b ?(a < c ? a : c) : (b < c ? b : c);
}

inline size_t nBytesCode(const char ch)
{
    if(ch & (1 << 7))//如果ch是多字节的，下面循环判断utf-8编码的长度
    {
        int nBytes = 1;
        for(int idx = 0; idx != 6; ++idx)
        {
            if(ch & (1 << (6 - idx)))
            {
                ++nBytes;
            } else
                break;
        }
        return nBytes;
    }
    return 1;
}

inline size_t length(const string & str)
{
    size_t ilen = 0;
    for(size_t idx = 0; idx != str.size(); ++idx)
    {
        int nBytes = nBytesCode(str[idx]);
        idx += (nBytes - 1);
        ++ilen;
    }
    return ilen;
}

void Mission::process()
{
    string key = cleanup_str(_msg);
    //从缓存中取结果
    string value =Redis::getInstance()->get(key);
    //如果缓存中没有,就通过索引新建结果,存入缓存
    if(value.empty()){
        if(key[0] >= 'a' && key[0] <= 'z')
            value = getEnResult(key);
        else
            value = getChResult(key);
        Redis::getInstance()->sett(key,value);
    }
    //通过eventLoop将send(msg)任务作为回调函数放到pendingFunctors中
    //并唤醒主线程去pendingFunctors执行send()回调函数
    //将msg在主线程通过该tcpConnection的socketIO发给客户端
    _conn->sendInLoop(value);
    //下面注释的是直接通过子线程的tcpConnection发信息给客户端
    //_conn->send(response);
    //由线程池的线程(计算线程)完成数据的发送,在设计上来说，是不合理的
	//数据发送的工作(send()函数)要交还给IO线程(Reactor所在的线程)完成
    cout<<" >> already response to client"<<endl;
}

inline string Mission::getEnResult(const string & source)
{ 
    size_t len = source.length();
    set<int> candidate_lineNo = _pIndex->alphIndex()[source[0]];
    //for(auto lineNo : candidate_lineNo) cout<<lineNo<<" ";
    //cout<<" >> 现在set<int> candidate_lineNo里有 -- "
    //    <<candidate_lineNo.size()<<" -- 个元素"<<endl;
    
    //当长度大于一个字母时需要借助临时容器求lineNo的交集
    if(len>1){
        set<int> temp;
        for(size_t i = 1; i != len; ++i){
            temp.clear();
            //求与后面字母的Index中lineNo的交集
            set<int> temp2 = _pIndex->alphIndex()[source[i]];
            set_intersection(temp2.begin(),temp2.end(),
                             candidate_lineNo.begin(),
                             candidate_lineNo.end(),
                             inserter(temp,temp.begin()));
            //若交集不空则赋给candidate_lineNo
            //空则保留上一个candidate_lineNo，否则返回空集没有意义
            if( !temp.empty() ) candidate_lineNo = temp;
            else break;
            //for(auto lineNo : candidate_lineNo) cout<<lineNo<<" ";
            //cout<<" >> 现在set<int> candidate_lineNo里有 -- "
            //    <<candidate_lineNo.size()<<" -- 个元素"<<endl;
        }
    }
    
    //candidate_lineNo对应的单词结构体入优先队列
    for(auto lineNo : candidate_lineNo){
        string candidate_word = _pDict->enDict()[lineNo].first;
        _que.push({editDistance(source,candidate_word),candidate_word
                  ,_pDict->enDict()[lineNo].second});
    }
    Json::Value root;
    Json::StyledWriter swriter;
    int num = _K;
    while( !_que.empty() && num > 0){
        root[" After correct >> "].append(_que.top().candidate_word.c_str());
        //cout<<" >> "<<_que.top().distanc<<" >> "<<_que.top().candidate_word
        //    <<" >> "<<_que.top().frequency<<endl;
        _que.pop(); --num; 
    }
    return swriter.write(root);
}

inline string Mission::getChResult(const string & source)
{ 
    size_t len = source.length();
    string zi = source.substr(0,3);
    set<int> candidate_lineNo = _pIndex->chIndex()[zi];
    for(auto lineNo : candidate_lineNo) cout<<lineNo<<" ";
    //cout<<" >> 现在set<int> candidate_lineNo里有 -- "
    //    <<candidate_lineNo.size()<<" -- 个元素"<<endl;
    
    //当长度大于一个汉字（占3个字节）时需要借助临时容器求lineNo的交集
    if(len>3){
        set<int> temp;
        for(size_t i = 1; i != len; ++i){
            temp.clear();
            zi = source.substr(3*i,3);
            //求与后面字母的Index中lineNo的交集
            set<int> temp2 = _pIndex->chIndex()[zi];
            set_intersection(temp2.begin(),temp2.end(),
                             candidate_lineNo.begin(),
                             candidate_lineNo.end(),
                             inserter(temp,temp.begin()));
            //若交集不空则赋给candidate_lineNo
            //空则保留上一个candidate_lineNo，否则返回空集没有意义
            if( !temp.empty() ) candidate_lineNo = temp;
            else break;
            for(auto lineNo : candidate_lineNo) cout<<lineNo<<" ";
            //cout<<" >> 现在set<int> candidate_lineNo里有 -- "
            //    <<candidate_lineNo.size()<<" -- 个元素"<<endl;
        }
    }
    
    //candidate_lineNo对应的单词结构体入优先队列
    for(auto lineNo : candidate_lineNo){
        string candidate_word = _pDict->chDict()[lineNo].first;
        _que.push({editDistance(source,candidate_word),candidate_word
                  ,_pDict->chDict()[lineNo].second});
    }
    
    Json::Value root;
    Json::StyledWriter swriter;
    int num = _K;
    while( !_que.empty() && num > 0){
        root[" After correct >> "].append(_que.top().candidate_word.c_str());
        //cout<<" >> "<<_que.top().distanc<<" >> "<<_que.top().candidate_word
        //    <<" >> "<<_que.top().frequency<<endl;
        _que.pop(); --num; 
    }
    return swriter.write(root);
}

inline int Mission::editDistance(const string & lhs, const string & rhs)
{
    size_t lhs_len = length(lhs);
    size_t rhs_len = length(rhs);
    size_t blhs_len = lhs.size();
    size_t brhs_len = rhs.size();

    int editDist[lhs_len + 1][rhs_len + 1];

    for(size_t idx = 0; idx <= lhs_len; ++idx)
    {
        editDist[idx][0] = idx;
    }
    for(size_t idx = 0; idx <= rhs_len; ++idx)
    {
        editDist[0][idx] = idx;
    }
    string sublhs, subrhs;
    for(size_t dist_i = 1, lhs_idx = 0; 
        dist_i <= lhs_len && lhs_idx <= blhs_len; ++dist_i, ++lhs_idx)
    {
        //cout << "lhs_idx = " << lhs_idx << endl;
        size_t nBytes = nBytesCode(lhs[lhs_idx]);
        sublhs = lhs.substr(lhs_idx, nBytes);
        lhs_idx += (nBytes - 1);

        for(size_t dist_j = 1, rhs_idx = 0; dist_j <= rhs_len && rhs_idx <= brhs_len; ++dist_j, ++rhs_idx)
        {
            //cout << "rhs_idx = " << lhs_idx << endl;
            nBytes = nBytesCode(rhs[rhs_idx]);
            subrhs = rhs.substr(rhs_idx, nBytes);
            rhs_idx += (nBytes - 1);
            if(sublhs == subrhs)
            {
                editDist[dist_i][dist_j] = editDist[dist_i - 1][dist_j - 1];
            } else {
                editDist[dist_i][dist_j] = triple_min(
                    editDist[dist_i][dist_j - 1] + 1,
                    editDist[dist_i - 1][dist_j] + 1,
                    editDist[dist_i - 1][dist_j - 1] + 1);
            }
        }
    }
    return editDist[lhs_len][rhs_len];
}
