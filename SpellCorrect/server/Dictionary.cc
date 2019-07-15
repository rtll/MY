#include"Dictionary.h"
#include<unistd.h>
#include<fcntl.h>
#include<fstream>
#include<string>
#include<sstream>
#include<algorithm>

Dictionary::Dictionary(string en_yuliao_path,string en_stop_path,string en_dict_path,
                       string ch_yuliao_paths,string ch_stop_path,string ch_dict_path)
:_en_yuliao_path(en_yuliao_path),_en_stop_path(en_stop_path)
,_en_dict_path(en_dict_path),_ch_yuliao_paths(ch_yuliao_paths)
,_ch_stop_path(ch_stop_path),_ch_dict_path(ch_dict_path),_wSeg()
{
    if((access(_en_dict_path.c_str(),F_OK)) == -1){
        cout<<"start make a en_dict file"<<endl;
        readYuliao();
        eraseStop();
        makeDict();
    }
    readDict();//read得到的vector单词按词频排序了
    //showDict();
    if((access(_ch_dict_path.c_str(),F_OK)) == -1){
        cout<<"start make a ch_dict file"<<endl;
        readChYuliaoPaths();
        eraseChStop();
        makeChDict();
    }
    readChDict();
    //showchDict();
}

inline string cleanup_str(const string & word)
{
    string ret;
    for(auto it : word){
        if(!ispunct(it)){
            if(it >= 'A' && it <= 'Z'){
                it += 32;
            }
            ret += it;
        }
    }
    return ret;
}

void Dictionary::readYuliao()
{
    ifstream ifs(_en_yuliao_path);
    if(!ifs){
        cout<<"open enYuliao_path error!"<<endl;
        return;
    }
    string line;
    while(getline(ifs,line)){
        istringstream istrs(line);
        string word;
        while(istrs>>word){
            if(word[0]<'A'||word[0]>'z'||(word[0]>'Z'&&word[0]<'a'))
                continue;
            word = cleanup_str(word);
            if(_dict_of_write.count(word)){
                ++_dict_of_write[word];
            }
            else{
                _dict_of_write[word] = 1;
            }
        }
    }
}
    
void Dictionary::eraseStop()
{
    ifstream ifs(_en_stop_path);
    if(!ifs){
        cout<<"open en_stop_path error!"<<endl;
        return;
    }
    string line;
    while(getline(ifs,line))
    {
        istringstream istrs(line);
        string word;
        while(istrs>>word){
            if(_dict_of_write.count(word)){
                _dict_of_write.erase(word);
            }
        }
    }
}

void Dictionary::makeDict()
{
    ofstream ofs(_en_dict_path);
    if(!ofs){
        cout<<"open en_dict ofstream error!"<<endl;
        return;
    }
    for(auto i : _dict_of_write){
        ofs<<i.first<<"   "<<i.second<<endl;
    }
}

void Dictionary::readDict()
{
    ifstream ifs(_en_dict_path);
    if(!ifs){
        cout<<"open en_dict ifstream error!"<<endl;
        return;
    }
    string line;
    while(getline(ifs,line)){
        istringstream istrs(line);
        string word;
        int frequency;
        while(istrs>>word>>frequency){
            _dict_of_read.push_back(make_pair(word,frequency));
        }
    }
    sort(_dict_of_read.begin(),_dict_of_read.end(),compare);
}

void Dictionary::showDict()
{
    auto idx = _dict_of_read.begin();
    for(int i = 0 ; i != 50 ;++i){
        cout<<idx->first<<"   "<<idx->second<<endl;
        ++idx;
    }
}

void Dictionary::readChYuliaoPaths()
{
    ifstream ifs(_ch_yuliao_paths);
    if(!ifs){
        cout<<"open /config/ch_yuliaos.txt file error!"<<endl;
    }
    string ch_yuliao_path;
    while(getline(ifs,ch_yuliao_path)){
        readChYuliao(ch_yuliao_path);
    }
}

void Dictionary::readChYuliao(const string & ch_yuliao_path)
{
    string art;
    ifstream ifss(ch_yuliao_path);
    if(!ifss){
        cout<<"open "<<ch_yuliao_path<<" error!"<<endl;
        return ;
    }
    string line;
    while(getline(ifss,line)){
        istringstream istrs(line);
        art += line;
    }
    vector<string> afterSeg = _wSeg.cut(art);
    int ch_flag; 
    for(auto& word : afterSeg){
        ch_flag = word[0];
        if(ch_flag >= 0 && ch_flag <= 127) continue;
        if(_Chdict_of_write.count(word)){
            ++_Chdict_of_write[word];
        }
        else{
            _Chdict_of_write[word] = 1;
        }
    }
}
    
void Dictionary::eraseChStop()
{
    ifstream ifs(_ch_stop_path);
    if(!ifs){
        cout<<"open ch_stop_path error!"<<endl;
        return;
    }
    string word;
    while(getline(ifs,word))
    {
        if(_Chdict_of_write.count(word)){
            _Chdict_of_write.erase(word);
        }
    }
}

void Dictionary::makeChDict()
{
    ofstream ofs(_ch_dict_path);
    if(!ofs){
        cout<<"open ch_dict ofstream error!"<<endl;
        return;
    }
    for(auto i : _Chdict_of_write){
        ofs<<i.first<<"   "<<i.second<<endl;
    }
}

void Dictionary::readChDict()
{
    ifstream ifs(_ch_dict_path);
    if(!ifs){
        cout<<"open ch_dict ifstream error!"<<endl;
        return;
    }
    string line;
    while(getline(ifs,line)){
        istringstream istrs(line);
        string word;
        int frequency;
        while(istrs>>word>>frequency){
            _Chdict_of_read.push_back(make_pair(word,frequency));
        }
    }
    sort(_Chdict_of_read.begin(),_Chdict_of_read.end(),compare);
}

void Dictionary::showchDict()
{
    auto idx = _Chdict_of_read.begin();
    for(int i = 0 ; i != 100 ;++i){
        cout<<idx->first<<"   "<<idx->second<<endl;
        ++idx;
    }
}
