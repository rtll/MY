#include"Index.h"
   
Index::Index(const vector<pair<string,int>> & enDict,string en_index_path,
             const vector<pair<string,int>> & chDict,string ch_index_path)
:_en_index_path(en_index_path),_ch_index_path(ch_index_path)
{
    if((access(_en_index_path.c_str(),F_OK)) == -1){
        for(auto idx : enDict){
            _enDict.push_back(idx.first);
        }
        //showEnDict();
        makeAlphIndex();
    }
    else{
        getAlphIndex();
    }
    //showAlphIndex();
    if((access(_ch_index_path.c_str(),F_OK)) == -1){
        for(auto idx : chDict){
            _chDict.push_back(idx.first);
        }
        cout<<"make ch_index start..."<<endl;
        makeChIndex();
    }
    else{
        getChIndex();
    }
}

void Index::makeAlphIndex(){
    int pos = 0;
    for(auto idx = _enDict.begin(); idx != _enDict.end(); ++idx){
        for(auto alph : *idx){
            if(alph >= 'a' && alph <= 'z'){
                if(!_alphIndex.count(alph)){
                    set<int> s;
                    s.clear();
                    s.insert(pos);
                    _alphIndex.insert(make_pair(alph,s));
                }
                else{
                    _alphIndex[alph].insert(pos);    
                }
            }
        } 
        ++pos;
    }
    ofstream ofs(_en_index_path);
    if(!ofs){
        cout<<"open en_index ofstream error!"<<endl;
        return;
    }
    for(auto i : _alphIndex){
        ofs<<i.first<<" ";
        for(auto j : i.second){
            ofs<<j<<" ";
        }
        ofs<<endl;
    }
}

void Index::getAlphIndex(){
    ifstream ifs(_en_index_path);
    if(!ifs){
        cout<<"open en_index ifstream error!"<<endl;
        return;
    }
    string line;
    char c;
    set<int> s;
    while(getline(ifs,line)){
        s.clear();
        istringstream istrs(line);
        string lineNo;
        while(istrs>>lineNo){
            if(lineNo[0] >= 'a' && lineNo[0] <= 'z'){
                c = lineNo[0];
                continue;
            }
            else{
                s.insert(stoi(lineNo));
            }
        }
        _alphIndex.insert(make_pair(c,s));
    }
}

void Index::showAlphIndex(){
    for(auto i : alphIndex()){
        cout<<" >> "<<i.first<<" : ";
        auto idx = i.second.begin();
        for(int j  = 0; j != 20 ; ++j){
            cout<<*idx<<" ";
            ++idx;
        }
        cout<<endl;
    }
}

void Index::showEnDict()
{
    auto idx = _enDict.begin();
    for(int i = 0 ; i != 50 ;++i){
        cout<<" >>> "<<*idx<<endl;
        ++idx;
    }
}

void Index::makeChIndex(){
    int pos = 0;
    string zi;
    for(auto idx = _chDict.begin(); idx != _chDict.end(); ++idx){
        int ch_size = (*idx).size();
        for(int start = 0 ; start < ch_size ; start += 3){
            //取三个字节为一个中文字
            zi = (*idx).substr(start,3);
            if(!_chIndex.count(zi)){
                set<int> s;
                s.clear();
                s.insert(pos);
                _chIndex.insert(make_pair(zi,s));
            }
            else{
                _chIndex[zi].insert(pos);    
            }
        } 
        ++pos;
    }
    ofstream ofs(_ch_index_path);
    if(!ofs){
        cout<<"open ch_index ofstream error!"<<endl;
        return;
    }
    for(auto i : _chIndex){
        ofs<<i.first<<" ";
        for(auto j : i.second){
            ofs<<j<<" ";
        }
        ofs<<endl;
    }
}

void Index::getChIndex(){
    ifstream ifs(_ch_index_path);
    if(!ifs){
        cout<<"open ch_index ifstream error!"<<endl;
        return;
    }
    string line;
    string zi;
    set<int> s;
    while(getline(ifs,line)){
        s.clear(); 
        istringstream istrs(line);
        string lineNo;
        while(istrs>>lineNo){
            if(lineNo[0] >= '0' && lineNo[0] <= '9'){
                s.insert(stoi(lineNo));
            }
            else{
                zi = lineNo;
                continue;
            }
        }
        _chIndex.insert(make_pair(zi,s));
    }
}
