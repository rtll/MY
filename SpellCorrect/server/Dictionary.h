#ifndef __Dictionary_H__ 
#define __Dictionary_H__
#include "jieba.h"
#include <iostream>
#include<map>
#include<vector>
using namespace std;

class Dictionary
{
public:
    
    static void destroy(){
        if(_pDictionay)
            delete _pDictionay;
    }

    static Dictionary* setInstanceDict(string a ,string b ,string c,
                                       string d ,string e, string f)
    {
        if(!_pDictionay){
            _pDictionay = new Dictionary(a,b,c,d,e,f);
        }
        atexit(destroy);
        return _pDictionay;
    }

    static Dictionary* getInstanceDict()
    {
        return _pDictionay;
    }
    
    vector<pair<string,int>> enDict() const { return _dict_of_read; }
    vector<pair<string,int>> chDict() const { return _Chdict_of_read; }
    
private:
    Dictionary(string,string,string,string,string,string);
    
    static bool compare(const pair<string,int>& p1,const pair<string,int>&p2){
        return p1.second>p2.second;
    }
    
    void readYuliao();
    void eraseStop();
    void makeDict();
    void readDict();
    void showDict();
    void readChYuliaoPaths();
    void readChYuliao(const string &);
    void eraseChStop();
    void makeChDict();
    void readChDict();
    void showchDict();
    string _en_yuliao_path;
    string _en_stop_path;
    string _en_dict_path;
    string _ch_yuliao_paths;
    string _ch_stop_path;
    string _ch_dict_path;
    map<string,int> _dict_of_write;
    vector<pair<string,int>> _dict_of_read;
    WordSegmentation _wSeg;
    map<string,int> _Chdict_of_write;
    vector<pair<string,int>> _Chdict_of_read;
    static Dictionary* _pDictionay;
};

#endif
