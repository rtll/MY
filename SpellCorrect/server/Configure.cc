#include"Configure.h"
#include<fstream>
#include<vector>
#include<string>

void Configure::readConfig()
{
    ifstream ifs(_config_path);
    if(!ifs){
        cout<<"open configure file error!"<<endl;
    }
    string line;
    vector<string> lines;
    lines.reserve(8);
    while(getline(ifs,line)){
        lines.push_back(line);
    }
    auto i = lines.begin();
    _ip = *i ; ++i ;
    _port = stoi(*i) ; ++i ;
    _enYuliao_path = *i ; ++i ;
    _chYuliao_path = *i ; ++i ;
    _enStop_path = *i ; ++i ;
    _chStop_path = *i ; ++i ;
    _enDict_path = *i ; ++i ;
    _chDict_path = *i ; ++i ;
    _enIndex_path = *i ; ++i ;
    _chIndex_path = *i ; ++i ;
}
