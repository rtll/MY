#ifndef __Index_H__ 
#define __Index_H__
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
using namespace std;

//set中的行号是按词频排序后的vector中单词的行号
//而非Dict.txt中的行号
class Index
{
public:

    static void destroy(){
        if(_pIndex)
            delete _pIndex;
    }

    static Index* setInstanceIndex(const vector<pair<string,int>> & a,string b,
                                   const vector<pair<string,int>> & c,string d)
    {
        if(!_pIndex){
            _pIndex = new Index(a,b,c,d);
        }
        atexit(destroy);
        return _pIndex;
    }

    static Index* getInstanceIndex()
    {
        return _pIndex;
    }

    void makeAlphIndex();
    void getAlphIndex();
    void showAlphIndex();
    void showEnDict();
    void makeChIndex();
    void getChIndex();

    unordered_map<char,set<int>> alphIndex() const { return _alphIndex; }
    unordered_map<string,set<int>> chIndex() const { return _chIndex; }
private:
    Index(const vector<pair<string,int>> & ,string,
          const vector<pair<string,int>> & ,string);
    string _en_index_path;
    vector<string> _enDict;
    unordered_map<char,set<int>> _alphIndex;
    string _ch_index_path;
    vector<string> _chDict;
    unordered_map<string,set<int>> _chIndex;
    static Index* _pIndex;
};

#endif

