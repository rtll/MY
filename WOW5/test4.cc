#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<string.h>
#include<iomanip>
#include<vector>
#include<map>
#include<memory>
#include<algorithm>
using namespace std;
using SPVST = shared_ptr<vector<size_t>>;

#define WEAPON_NUM 10

enum WeaponType{
    SWORD,
    BOMB,
    ARROW
};

enum WarriorType{
    DRAGON,
    NINJA,
    ICEMAN,
    LION,
    WOLF
};

class Warrior;

class GameTime
{
public:
    static GameTime* getInstance(){
        if(!_pInstance){
            _pInstance = new GameTime;
            atexit(destroy);
        }
        return _pInstance;
    }

    static void destroy(){
        if(_pInstance)
            delete _pInstance;
    }

    void showTime() const{
        cout<<setw(3)<<setfill('0')<<_hour<<":"
            <<setw(2)<<setfill('0')<<_minute<<" ";
    }

    void update_minute(size_t minute){ _minute = minute; }
    void add_hour(){ _hour++; }
    void reset() { _hour = 0; _minute = 0; }
    bool unsafe(size_t add,size_t deadline){
        return ((_hour * 60 + _minute + add) > deadline)? true : false; 
    }
private:
    GameTime(size_t hour = 0,size_t minute = 0)
        :_hour(hour),_minute(minute)
        {}
    ~GameTime(){}

    static GameTime* _pInstance;
    size_t _hour;
    size_t _minute;
};

GameTime* GameTime::_pInstance = nullptr; 

class City
{
public:
    City(size_t No)
        :_No(No),_warrior1(nullptr),_warrior2(nullptr),_pTime(GameTime::getInstance())
    {}

    ~City(){}

    void attach1(Warrior* w1){
        _warrior1 = w1;
    }
    void attach2(Warrior* w2){
        _warrior2 = w2;
    }

    void detach1(){
        _warrior1 = nullptr;
    }
    void detach2(){
        _warrior2 = nullptr;
    }

    size_t City_No() const { return _No; }
    Warrior* warrior1(){ return _warrior1; }
    Warrior* warrior2(){ return _warrior2; }

private:
    size_t _No;
    Warrior* _warrior1;
    Warrior* _warrior2;
    GameTime* _pTime;
};

class Config
{
    friend class War;
public:
    static Config* getInstance(){
        return _pConfig;
    }

    static void destroy(){
        if(_pConfig)
            delete _pConfig;
    }

    //该函数里进行配置战场信息
    static Config* setInstance(SPVST el,SPVST sl,size_t city_num){
        _pConfig = new Config(el,sl,city_num);
        atexit(destroy);
        return _pConfig;
    }

    string Camp1_name(){ return _camp1_name; }
    string Camp2_name(){ return _camp2_name; }
    SPVST Camp1_order(){ return _camp1_order; }
    SPVST Camp2_order(){ return _camp2_order; }
    SPVST Elemen_list(){ return _element_list; }
    SPVST Strength_list(){ return _strength_list; }
    size_t City_num(){ return _city_num; }
    shared_ptr<vector<City*>> Citys(){ return _citys; }
private:
    Config(SPVST el,SPVST sl, size_t city_num)
        :_camp1_order(make_shared<vector<size_t>>())
        ,_camp2_order(make_shared<vector<size_t>>())
        ,_element_list(el),_strength_list(sl)
        ,_camp1_name("red"),_camp2_name("blue"),_city_num(city_num)
        ,_citys(make_shared<vector<City*>>())
        {
            _camp1_order->insert(_camp1_order->end(),{ICEMAN,LION,WOLF,NINJA,DRAGON});
            _camp2_order->insert(_camp2_order->end(),{LION,DRAGON,NINJA,ICEMAN,WOLF});
            for(size_t i = 0 ; i != _city_num ; ++i){
                _citys->push_back(new City(i+1));
            }
        }
    ~Config(){}

    static Config* _pConfig;
    SPVST _camp1_order;
    SPVST _camp2_order;
    SPVST _element_list;
    SPVST _strength_list;
    string _camp1_name;
    string _camp2_name;
    size_t _city_num;
    shared_ptr<vector<City*>> _citys;
};

Config* Config::_pConfig = nullptr; 

class Weapon
{
public:
    Weapon(size_t weapon_No,size_t warrior_strength)
        :_weapon_No(weapon_No)
    {
        switch(_weapon_No){
        case 0:
            _weapon_name = "sword";_attack_times = 1;
            _strength = size_t((warrior_strength)*2/10);break;
        case 1:
            _weapon_name = "bomb";_attack_times = 1;
            _strength = size_t((warrior_strength)*4/10);break;
        case 2:
            _weapon_name = "arrow";_attack_times = 2;
            _strength = size_t((warrior_strength)*3/10);break;
        }
    }
   
    void reset_strength(size_t warrior_strength){
        switch(_weapon_No){
        case 0:
            _strength = size_t((warrior_strength)*2/10);break;
        case 1:
            _strength = size_t((warrior_strength)*4/10);break;
        case 2:
            _strength = size_t((warrior_strength)*3/10);break;
        }
    }

    void use(){
        if(_weapon_No != SWORD)
            _attack_times --;
    }

    size_t get_No() const { return _weapon_No; }
    size_t get_Strength() const { return _strength; }
    string show_weapon() const { return _weapon_name;}
    size_t rest_times() const { return _attack_times; }
private:
    size_t _weapon_No;
    size_t _strength;
    string _weapon_name;
    size_t _attack_times;
};

bool arrow_sort_before_rape(Weapon* w1 , Weapon* w2){
    //降序：没用过的排前面
    return w1->rest_times() > w2->rest_times();
}

bool arrow_sort_before_fight(Weapon* w1 , Weapon* w2){
    //升序：用过的排前面
    return w1->rest_times() < w2->rest_times();
}

class Warrior
{
public:
    Warrior(size_t e,size_t s,size_t noc,string species,string f)
        :_weapons(new multimap<size_t,Weapon*>)
         ,_element(e),_strength(s),_No_of_camp(noc),_species(species),_flag(f)
         ,_pTime(GameTime::getInstance()),_pConfig(Config::getInstance())
         ,_No_of_westCamp(0),_No_of_eastCamp(_pConfig->City_num() + 1)
         ,_round_flag(0)
    {
        _pTime->showTime();
        cout<<_flag<<" "<<_species<<" "<<_No_of_camp<<" born"<<endl;
        //红方武士出生城市编号为0，即最西边红军司令部
        //蓝方武士出生城市编号为城市数量+1，即最东边蓝军司令部
        if(_flag == _pConfig->Camp1_name()){
            _city_No = _No_of_westCamp;
        }
        else if(_flag == _pConfig->Camp2_name()){
            _city_No = _No_of_eastCamp;
        }
    }

    string flag() const { return _flag; }
    string species() const { return _species; }
    shared_ptr<multimap<size_t,Weapon*>> weapons() { return _weapons; }
    size_t No_of_camp() const { return _No_of_camp; }
    size_t No_of_city() const { return _city_No; }
    size_t HP() const { return _element; }
    size_t MP() const { return _strength; }

    void reduce_element(size_t reduction){
        _element -= reduction; 
    }

    virtual int escape() = 0;
    virtual void set_weapon() = 0;
    virtual void yelling() = 0;
    virtual void reduceHP_perStep() = 0;
    virtual void reduceLoyalty_perStep() = 0;

    Warrior* march(){
        //将武士从当前城市撤出
        //若是在司令部的武士则不用做此操作
        if(_city_No != _No_of_westCamp && _city_No != _No_of_eastCamp){
            if(_flag == _pConfig->Camp1_name()){
                (*_pConfig->Citys())[_city_No-1]->detach1();
            }
            else if(_flag == _pConfig->Camp2_name()){
                (*_pConfig->Citys())[_city_No-1]->detach2();
            }
        }
        //仅iceman扣生命值的行为
        reduceHP_perStep();
        //仅lion扣忠诚度的行为
        reduceLoyalty_perStep();

        //更改所在城市编号
        if(_flag == _pConfig->Camp1_name())
        {
            ++ _city_No;
            //若红军到达东边司令部
            if(_city_No == _No_of_eastCamp){
                return this;
            }
            //若非进入敌军司令部地进入下一个城市
            (*_pConfig->Citys())[_city_No-1]->attach1(this);
        }
        else if(_flag == _pConfig->Camp2_name()){
            -- _city_No;
            //若蓝军到达西边司令部
            if(_city_No == _No_of_westCamp){
                return this;
            }
            (*_pConfig->Citys())[_city_No-1]->attach2(this);
        }
        return nullptr;
    }

    //wolf只能抢一种武器，不同于战斗结束的收缴不受种类限制
    void wolf_rape() {
        //若武士非Wolf或者自己的武器库已满则退出
        if( _species != "wolf" || _weapons->size() == WEAPON_NUM) return;
        Warrior* enemy = nullptr;
        //通过配置类的citys获取位于相同城市的敌人
        if( _flag == _pConfig->Camp1_name() ){
            enemy = (*_pConfig->Citys())[_city_No-1]->warrior2(); 
        }
        else if( _flag == _pConfig->Camp2_name() ){
            enemy = (*_pConfig->Citys())[_city_No-1]->warrior1();
        }
        //若所在城市没有敌人或敌人也是Wolf或敌人没有武器则退出
        if( enemy == nullptr || enemy->_species == "wolf" || enemy->_weapons->empty()) 
            return;

        //计算最多还能抢多少个武器
        size_t weapon_space = WEAPON_NUM - _weapons->size();
        //计算敌人编号最小的武器有多少个,由map的特性键值最小的在begin()位置
        auto it = enemy->weapons()->begin();
        size_t min_No = it->first;
        size_t min_weapon_num = enemy->weapons()->count(min_No);
        auto temp = it;
        size_t n;
        //如果武器空位多于能抢的数量
        if(weapon_space >= min_weapon_num){
            for(size_t i = 0;i != min_weapon_num;++i){
                //复制到自己的武器库中
                _weapons->insert(make_pair(it->first,it->second));
                ++temp;
            }
            //从敌人的武器库中抹去
            enemy->weapons()->erase(min_No);
            n = min_weapon_num;
        }
        //若武器空位少于能抢的数量且能抢的武器不是Arrow
        else if( min_No != ARROW){
            for(size_t i = 0;i != weapon_space;++i){
                _weapons->insert(make_pair(it->first,it->second));    
                ++temp;
            }
            enemy->weapons()->erase(it,temp);
            n = weapon_space;
        }
        //若武器空位少于能抢的Arrow数量，则先抢或抢完剩余次数为2的
        //因为Arrow的键值最大，故该情况下武器multimap中所有的武器都是Arrow
        else{
            //直接对multimap的piar(size_t,Weapon*)排序会报错
            //应该将它们转移到vector中对剩余使用次数排序
            shared_ptr<vector<Weapon*>> temp_arrows(new vector<Weapon*>);
            for(auto i : *(enemy->weapons())){
                temp_arrows->push_back(i.second);
            }
            sort(temp_arrows->begin(),temp_arrows->end(),arrow_sort_before_rape);            
            //将排序后靠前的武器（剩余使用次数大的）转移到自己的武器库
            for(size_t i = 0;i != weapon_space;++i){
                _weapons->insert(make_pair(ARROW,(*temp_arrows)[i]));
            }
            //先清空敌人的武器库，再添加排序后靠后的武器
            //表示被掠夺后的剩余武器
            enemy->weapons()->clear();
            for(size_t j = weapon_space;j != temp_arrows->size();++j){
                enemy->weapons()->insert(make_pair(ARROW,(*temp_arrows)[j]));
            }
            n = weapon_space;
        }

        //将抢来的武器的伤害适配自身攻击力
        for(auto j : *_weapons){
            j.second->reset_strength(_strength);
        }

        _pTime->showTime();
        cout<<flag()<<" "<<species()<<" "<<No_of_camp()<<" took "
            <<min_weapon_num<<" "<<it->second->show_weapon()<<" from "
            <<enemy->flag()<<" "<<enemy->species()<<" "<<enemy->No_of_camp()
            <<" in city "<<No_of_city()<<endl;
    }

    void weapon_sort_before_fight(shared_ptr<multimap<size_t,Weapon*>> w){
        //因为multimap已对武器编号进行排序
        //故只需对有一个以上Arrow的武士对其使用次数排序
        if(w->count(ARROW) > 1){
            shared_ptr<vector<Weapon*>> temp_weapons(new vector<Weapon*>);
            //转移到vector排序
            for(auto i = w->begin();i != w->end();){
                if(i->first == ARROW){
                    temp_weapons->push_back(i->second);
                    //从自己的武器库消除
                    i = w->erase(i);
                }
                else ++i;
            }
            sort(temp_weapons->begin(),temp_weapons->end(),arrow_sort_before_fight);
            //将排序过的转移回来
            for(auto j : *temp_weapons){
                w->insert(make_pair(ARROW,j));
            }
        }
    }

    void weapon_sort_before_rape(shared_ptr<multimap<size_t,Weapon*>> w){
        if(w->count(ARROW) > 1){
            shared_ptr<vector<Weapon*>> temp_weapons(new vector<Weapon*>);
            for(auto i = w->begin();i != w->end();){
                if(i->first == ARROW){
                    temp_weapons->push_back(i->second);
                    i = w->erase(i);
                }
                else ++i;
            }
            sort(temp_weapons->begin(),temp_weapons->end(),arrow_sort_before_rape);
            //将排序过的转移回来
            for(auto j : *temp_weapons){
                w->insert(make_pair(ARROW,j));
            }
        }
    }
    
    int attack(Warrior* me,multimap<size_t,Weapon*>::iterator weapon_pos,Warrior* enemy){
        //当当前武士没有武器用完两轮或当前武器没有使用次数则无法攻击，直接退出
        if(weapon_pos == me->weapons()->end() || !weapon_pos->second->rest_times()){
            return 0;
        }
        //本方扣掉武器使用次数
        weapon_pos->second->use();
        //造成对方扣血,若伤害大于等于对方HP则对方HP直接归零
        if(weapon_pos->second->get_Strength() >= enemy->HP())
            enemy->reduce_element(enemy->HP());
        else
            enemy->reduce_element(weapon_pos->second->get_Strength());
        //非ninja武士使用炸弹时会自己扣血
        if( me->species() != "ninja" && weapon_pos->first == BOMB){
            if((size_t)(weapon_pos->second->get_Strength()/2) >= me->HP())
                me->reduce_element(me->HP());
            else
                me->reduce_element((size_t)(weapon_pos->second->get_Strength()/2));
        }
#if 0
        cout<<" <<< "<<me->flag()<<" "<<me->species()<<" "<<me->No_of_camp()<<" use "
            <<weapon_pos->second->show_weapon()<<" hurt "<<enemy->flag()
            <<" "<<enemy->species()<<" "<<enemy->No_of_camp()<<" "
            <<weapon_pos->second->get_Strength()<<" , weapon remain "
            <<weapon_pos->second->rest_times()<<" times,"<<endl
            <<"  << me remain "<<me->HP()<<" HP and enemy remain "
            <<enemy->HP()<<" HP"<<endl;
#endif
        if(me->_element == 0 && enemy->_element > 0)return -1;
        else if(me->_element > 0 && enemy->_element == 0)return 1;
        else if(me->_element == 0 && enemy->_element == 0)return 2;
        else return 0;
    }

    void show_result(Warrior* me,Warrior* enemy,int flag){
        Warrior* redw = me->flag() == _pConfig->Camp1_name() ? me : enemy;
        Warrior* bluew = me->flag() == _pConfig->Camp2_name() ? me : enemy;
        switch(flag){
        //自己死了（炸弹）
        case -1:
            _pTime->showTime();
            cout<<enemy->_flag<<" "<<enemy->_species<<" "<<enemy->_No_of_camp
                <<" killed "<<me->_flag<<" "<<me->_species<<" "<<me->_No_of_camp
                <<" in city "<<me->_city_No<<" remaining "<<enemy->_element<<" elements"<<endl;
            break;
        //都存活
        case 0:
            _pTime->showTime();
            cout<<"both "<<redw->_flag<<" "<<redw->_species<<" "<<redw->_No_of_camp
                <<" and "<<bluew->_flag<<" "<<bluew->_species<<" "<<bluew->_No_of_camp
                <<" were alive in city "<<me->_city_No<<endl;
            break;
        //对手死了
        case 1:
            _pTime->showTime();
            cout<<me->_flag<<" "<<me->_species<<" "<<me->_No_of_camp
                <<" killed "<<enemy->_flag<<" "<<enemy->_species<<" "<<enemy->_No_of_camp
                <<" in city "<<me->_city_No<<" remaining "<<me->_element<<" elements"<<endl;
            break;
        //都死了
        case 2:
            _pTime->showTime();
            cout<<"both "<<redw->_flag<<" "<<redw->_species<<" "<<redw->_No_of_camp
                <<" and "<<bluew->_flag<<" "<<bluew->_species<<" "<<bluew->_No_of_camp
                <<" died in city "<<me->_city_No<<endl;
            break;
        }
    }
    
    void report(){
        _pTime->showTime();
        cout<<_flag<<" "<<_species<<" "<<_No_of_camp<<" has "<<_weapons->count(SWORD)
            <<" sword "<<_weapons->count(BOMB)<<" bomb and "<<_weapons->count(ARROW)
            <<" arrow and "<<_element<<" elements"<<endl;
    }

    //此函数只由先攻击的一方执行，在War中判断并调用
    void fighting(Warrior* enemy){
        weapon_sort_before_fight(_weapons);
        weapon_sort_before_fight(enemy->_weapons);
        auto pos = _weapons->begin();
        auto enemy_pos = enemy->_weapons->begin();
        int flag1,flag2,times1,times2;
        while(1){
            flag1 = flag2 = times1 = times2 = 0;
            flag1 = attack(this,pos,enemy);
            if(!flag1){ //当前武士攻击未出战果
                if( pos != _weapons->end()){
                    ++pos;
                }
                if( pos == _weapons->end()){
                    pos = _weapons->begin();
                }
                flag2 = attack(enemy,enemy_pos,this);
                if(!flag2){//对方武士攻击未出战果
                    if( enemy_pos != enemy->_weapons->end()){
                        ++enemy_pos;
                    }
                    if( enemy_pos == enemy->_weapons->end()){
                        enemy_pos = enemy->_weapons->begin();               
                    }
                    //统计是否还有没用完的武器
                    for(auto weapon1 : *_weapons){
                        //若该武器的伤害为0则视为已用完，不计入
                        if(!weapon1.second->get_Strength())
                            continue;
                        times1 += weapon1.second->rest_times();
                    }
                    for(auto weapon2 : *(enemy->_weapons)){
                        if(!weapon2.second->get_Strength())
                            continue;
                        times2 += weapon2.second->rest_times();
                    }
                    //平局出口，当双方的武器都用完且并没有从战斗结束的出口break
                    if( !times1 && !times2 ){
                        show_result(this,enemy,0);  break;
                    }
                }
                else{//对方武士攻击导致战斗结束
                    show_result(enemy,this,flag2);  break;
                }
            }
            else{//当前武士攻击导致战斗结束
                show_result(this,enemy,flag1);  break;
            }
        }

        //将双方武器中使用次数为0的武器消去
        for(auto it = _weapons->begin();it != _weapons->end();){
            if(it->second->rest_times() == 0){
                it = _weapons->erase(it);
            }
            else ++it;
        }
        for(auto it = enemy->_weapons->begin();it != enemy->_weapons->end();){
            if(it->second->rest_times() == 0){
                it = enemy->_weapons->erase(it);
            }
            else ++it;
        }
        
        //本方武士胜利，掠夺对手武器
        if(flag1 == 1 || flag2 == -1){
            //只有Dragon才实现的欢呼
            this->yelling();
            //当若自己武器栏没有空位或对方没有武器，退出
            if(_weapons->size() == WEAPON_NUM || !enemy->_weapons->size()) return;
            size_t rape_num = ( WEAPON_NUM - _weapons->size() <= enemy->_weapons->size())?
                                WEAPON_NUM - _weapons->size() : enemy->_weapons->size();
            weapon_sort_before_rape(enemy->_weapons);
            auto it = enemy->_weapons->begin();
            for(size_t i = 0;i != rape_num;++i){
                it = enemy->_weapons->begin();
                _weapons->insert(make_pair(it->first,it->second));
                enemy->_weapons->erase(it);
            }
            //重新将武器的伤害适配自身攻击力
            for(auto j : *_weapons){
                j.second->reset_strength(_strength);
            }
        }   
        //对方武士胜利，掠夺本方武器
        else if(flag1 == -1 || flag2 == 1){
            enemy->yelling();
            if(enemy->_weapons->size() == WEAPON_NUM || !_weapons->size()) return;
            size_t rape_num = ( WEAPON_NUM - enemy->_weapons->size() <= _weapons->size())?
                                WEAPON_NUM - enemy->_weapons->size() : _weapons->size();
            weapon_sort_before_rape(_weapons);
            auto it = _weapons->begin();
            for(size_t i = 0;i != rape_num;++i){
                it = _weapons->begin();
                enemy->_weapons->insert(make_pair(it->first,it->second));
                _weapons->erase(it);
            }
            for(auto j : *(enemy->_weapons)){
                j.second->reset_strength(enemy->_strength);
            }
        }
        //平局，双方存活dragon的欢呼
        else if( flag1 == 0 || flag2 ==0 ){
            this->yelling();
            enemy->yelling();
            return;
        }
    }

    virtual ~Warrior(){}
private:
    shared_ptr<multimap<size_t,Weapon*>> _weapons;
    size_t _element;
    size_t _strength;
    size_t _No_of_camp;
    string _species;
    string _flag;
    size_t _city_No;
    GameTime* _pTime;
    Config* _pConfig;
    size_t _No_of_westCamp;
    size_t _No_of_eastCamp;
    size_t _round_flag;
};

class Dragon
:public Warrior
{
public:
    Dragon(size_t e,size_t s,size_t noc,string species,string f)
        :Warrior(e,s,noc,species,f)
    {}

    void set_weapon() override {
        weapons()->insert(make_pair(No_of_camp()%3,new Weapon(No_of_camp()%3,MP()))); 
    }

    void yelling() override {
        GameTime::getInstance()->showTime();
        cout<<flag()<<" dragon "<<No_of_camp()
            <<" yelled in city "<<No_of_city()<<endl;
    }

    int escape() override { return 0; };
    void reduceHP_perStep() override{}
    void reduceLoyalty_perStep() override {}
private:
};

class Ninja
:public Warrior
{
public:
    Ninja(size_t e,size_t s,size_t noc,string species,string f)
        :Warrior(e,s,noc,species,f)
    {}

    void set_weapon() override { 
        weapons()->insert(make_pair(No_of_camp()%3,new Weapon(No_of_camp()%3,MP()))); 
        weapons()->insert(make_pair((No_of_camp()+1)%3,new Weapon((No_of_camp()+1)%3,MP()))); 
    }

    int escape() override { return 0; };
    void yelling() override {}
    void reduceHP_perStep() override {}
    void reduceLoyalty_perStep() override {}
private:
};

class Iceman
:public Warrior
{
public:
    Iceman(size_t e,size_t s,size_t noc,string species,string f)
        :Warrior(e,s,noc,species,f)
    {}

    void set_weapon() override {
        weapons()->insert(make_pair(No_of_camp()%3,new Weapon(No_of_camp()%3,MP()))); 
    }

    void reduceHP_perStep() override {
        reduce_element(size_t(HP()/10));    
    }

    int escape() override { return 0; };
    void yelling() override {}
    void reduceLoyalty_perStep() override {}
private:
};

class Lion
:public Warrior
{
public:
    Lion(size_t e,size_t s,size_t noc,string species,string f,size_t he,size_t K)
        :Warrior(e,s,noc,species,f),_loyalty(he),_K(K)
    {
        cout<<"Its loyalty is "<<_loyalty<<endl;
    }

    void set_weapon() override {
        weapons()->insert(make_pair(No_of_camp()%3,new Weapon(No_of_camp()%3,MP()))); 
    }

    int escape() override {
        if(_loyalty <= 0){
            GameTime::getInstance()->showTime();
            cout<<flag()<<" lion "<<No_of_camp()<<" ran away"<<endl;
            return -1;
        }
        return 0;
    };

    void reduceLoyalty_perStep() override {
        _loyalty -= _K;
    }

    void yelling() override {}
    void reduceHP_perStep() override {}
private:
    size_t _loyalty;
    size_t _K;
};

class Wolf
:public Warrior
{
public:
    Wolf(size_t e,size_t s,size_t noc,string species,string f)
        :Warrior(e,s,noc,species,f)
    {}

    void set_weapon() override {}
    void reduceLoyalty_perStep() override {}
    int escape() override { return 0; };
    void yelling() override {}
    void reduceHP_perStep() override {}
private:
};


class Camp
{
friend class War;
public:
    Camp(size_t n,size_t headquarter_element,size_t royalty_decrement,SPVST el,SPVST sl)
        :_Warrior_num(0),_headquarter_element(headquarter_element)
         ,_royalty_decrement(royalty_decrement),_order(0)
         ,_element_list(el),_strength_list(sl)
         ,_make_order(make_shared<vector<size_t>>())
         ,_army(make_shared<map<size_t,Warrior*>>())
         ,_min_element(*min_element(_element_list->begin(),_element_list->end()))
    {
        if(!n){
            _flag = Config::getInstance()->Camp1_name();
            _make_order = Config::getInstance()->Camp1_order();
        }
        else{
            _flag = Config::getInstance()->Camp2_name();
            _make_order = Config::getInstance()->Camp2_order();
        }
    }

    void ready(Warrior* w){
        //为他配置初始武器
        w->set_weapon();
        //将该武士编入军队map中
        _army->insert(make_pair(_Warrior_num,w));
    }

    void make_dragon(){
        _headquarter_element -= (*_element_list)[DRAGON];
        Warrior* w = new Dragon((*_element_list)[DRAGON],(*_strength_list)[DRAGON]
                                ,++_Warrior_num,"dragon",_flag);
        ready(w);
    }

    void make_ninja(){
        _headquarter_element -= (*_element_list)[NINJA];
        Warrior* w = new Ninja((*_element_list)[NINJA],(*_strength_list)[NINJA]
                               ,++_Warrior_num,"ninja",_flag);
        ready(w);
    }

    void make_iceman(){
        _headquarter_element -= (*_element_list)[ICEMAN];
        Warrior* w = new Iceman((*_element_list)[ICEMAN],(*_strength_list)[ICEMAN]
                                ,++_Warrior_num,"iceman",_flag);
        ready(w);
    }

    void make_lion(){
        _headquarter_element -= (*_element_list)[LION];
        Warrior* w = new Lion((*_element_list)[LION],(*_strength_list)[LION]
                              ,++_Warrior_num,"lion",_flag
                              ,_headquarter_element,_royalty_decrement);
        ready(w);
    }

    void make_wolf(){
        _headquarter_element -= (*_element_list)[WOLF];
        Warrior* w = new Wolf((*_element_list)[WOLF],(*_strength_list)[WOLF]
                              ,++_Warrior_num,"wolf",_flag);
        ready(w);
    }

    void make_Warrior(){
        if(_order == _make_order->size()){
            _order = 0;
        }
        //如果司令部剩余生命值不大于制造当前武士所需的生命值
        //则视为无法再制造
        if(_headquarter_element <= (*_element_list)[(*_make_order)[_order]]){
            return;
        }
        switch((*_make_order)[_order]){
        case DRAGON:
            make_dragon(); ++_order; break;
        case NINJA:
            make_ninja(); ++_order; break;
        case ICEMAN:
            make_iceman(); ++_order; break;
        case LION:
            make_lion(); ++_order; break;
        case WOLF:
            make_wolf(); ++_order; break;
        }    
    }

    Warrior* march(){
        Warrior* winner = nullptr;
        Warrior* warrior = nullptr;
        for(auto i : *_army){
            warrior = nullptr;
            warrior = i.second->march();
            if(warrior != nullptr){
                //若有一方占领另一方司令部则返回占领者的指针
                //则可告知War类终止战斗
                winner = warrior;
            }
        }
        delete warrior;
        return winner;
    }

    void camp_repot(){
        GameTime::getInstance()->showTime();
        cout<<_headquarter_element<<" elements in "<<_flag<<" headquarter"<<endl;
    }

private:
    string _flag;
    size_t _Warrior_num;
    size_t _headquarter_element;
    size_t _royalty_decrement;
    size_t _order;
    SPVST _element_list;
    SPVST _strength_list;
    SPVST _make_order;
    shared_ptr<map<size_t,Warrior*>> _army;
    size_t _min_element;
};

class War
{
public:
    War(size_t headquarter_element,size_t city_num
        ,size_t royalty_decrement,size_t deadline
        ,SPVST element_list,SPVST strength_list)
        :_deadline(deadline),_ptime(GameTime::getInstance())
        ,_pconfig(Config::setInstance(element_list,strength_list,city_num))
        ,_A(make_shared<Camp>(0,headquarter_element,royalty_decrement,element_list,strength_list))
        ,_B(make_shared<Camp>(1,headquarter_element,royalty_decrement,element_list,strength_list))
    {
        _ptime->reset();
    }

    void fight_in_city(){
        //i的类型是City*
        for(auto i : *_pconfig->_citys){
            //若城市中没有两个武士则跳过
            if(i->warrior1() == nullptr || i->warrior2() == nullptr)
                continue;
            //奇数城市红方武士先攻击
            if(i->City_No()%2){
                i->warrior1()->fighting(i->warrior2());
            }
            //偶数城市蓝方武士先攻击
            else{
                i->warrior2()->fighting(i->warrior1());
            }
            //将死亡武士消去
            if(i->warrior1()->HP() == 0){
                _A->_army->erase(i->warrior1()->No_of_camp());
                i->detach1();
            }
            if(i->warrior2()->HP() == 0){
                _B->_army->erase(i->warrior2()->No_of_camp());
                i->detach2();
            }
        }
    }

    void lion_escape(){
        for(auto i : *_pconfig->_citys){
            //若Lion类判断忠诚度后返回-1
            //则从该军军队和所在城市中删除该武士信息表示已逃走
            if(i->warrior1()){
                if(i->warrior1()->escape() == -1){
                    _A->_army->erase(i->warrior1()->No_of_camp());
                    i->detach1();
                }
            }
            if(i->warrior2()){
                if(i->warrior2()->escape() == -1){
                    _B->_army->erase(i->warrior2()->No_of_camp());
                    i->detach2();
                }
            }
        }
    }

    int march(){
        int over_flag = 0;
        Warrior* A_winner = _A->march();
        Warrior* B_winner = _B->march();
        //因为行军情况从西往东打印,所以先判断蓝军是否抵达西边司令部
        if(B_winner){
            over_flag = 1;
            _ptime->showTime();
            cout<<_pconfig->Camp2_name()<<" "<<B_winner->species()<<" "
                <<B_winner->No_of_camp()<<" reached "<<_pconfig->Camp1_name()
                <<" headquarter with "<<B_winner->HP()<<" elements and force "
                <<B_winner->MP()<<endl;
            _ptime->showTime();
            cout<<_pconfig->Camp1_name()<<" headquarter was taken"<<endl;
        }
        //再按城市顺序打印每个城市的行军情况，最后判断红军是否抵达东边司令部
        for(auto i : *_pconfig->_citys){
            if(i->warrior1()){
                _ptime->showTime();
                cout<<i->warrior1()->flag()<<" "<<i->warrior1()->species()<<" "
                    <<i->warrior1()->No_of_camp()<<" marched to city "
                    <<i->warrior1()->No_of_city()<<" with "<<i->warrior1()->HP()
                    <<" elements and force "<<i->warrior1()->MP()<<endl;
            }
            if(i->warrior2()){
                _ptime->showTime();
                cout<<i->warrior2()->flag()<<" "<<i->warrior2()->species()<<" "
                    <<i->warrior2()->No_of_camp()<<" marched to city "
                    <<i->warrior2()->No_of_city()<<" with "<<i->warrior2()->HP()
                    <<" elements and force "<<i->warrior2()->MP()<<endl;
            }
        }
        //最后判断红军是否抵达东边司令部
        if(A_winner){
            over_flag = 1;
            _ptime->showTime();
            cout<<_pconfig->Camp1_name()<<" "<<A_winner->species()<<" "
                <<A_winner->No_of_camp()<<" reached "<<_pconfig->Camp2_name()
                <<" headquarter with "<<A_winner->HP()<<" elements and force "
                <<A_winner->MP()<<endl;
            _ptime->showTime();
            cout<<_pconfig->Camp2_name()<<" headquarter was taken"<<endl;
        }
        return over_flag;
    }

    void wolf_rape(){
        for(auto i : *_pconfig->_citys){
            if(i->warrior1())
                i->warrior1()->wolf_rape();
            if(i->warrior2())
                i->warrior2()->wolf_rape();
        }
    }
    
    void warrior_report(){
        for(auto i : *_pconfig->_citys){
            if(i->warrior1())
                i->warrior1()->report();
            if(i->warrior2())
                i->warrior2()->report();
        }
    }

    void make_war(){
        while(1){
            _A->make_Warrior();
            _B->make_Warrior();

            if(_ptime->unsafe(5,_deadline)) break;
            _ptime->update_minute(5);
            lion_escape();

            if(_ptime->unsafe(5,_deadline)) break;
            _ptime->update_minute(10);
            if( march() ) break;

            if(_ptime->unsafe(20,_deadline)) break;
            _ptime->update_minute(35);
            wolf_rape();

             if(_ptime->unsafe(5,_deadline)) break;
            _ptime->update_minute(40);
            fight_in_city();

            if(_ptime->unsafe(10,_deadline)) break;
            _ptime->update_minute(50);
            _A->camp_repot();
            _B->camp_repot();

            if(_ptime->unsafe(5,_deadline)) break;
            _ptime->update_minute(55);
            warrior_report();

            if(_ptime->unsafe(5,_deadline)) break;
            _ptime->update_minute(0);
            _ptime->add_hour();
        }
    }

private:
    size_t _deadline;
    GameTime* _ptime;
    Config* _pconfig;
    shared_ptr<Camp> _A;
    shared_ptr<Camp> _B;
};

int main()
{
    size_t headquarter_element;
    size_t city_num;
    size_t royalty_decrement;
    size_t deadline;
    vector<vector<size_t>> config_map;
    vector<vector<size_t>> element_map;
    vector<vector<size_t>> strength_map;
    vector<size_t> config_list;
    vector<size_t> element_list;
    vector<size_t> strength_list;
    size_t test_num,i,dragon,ninja,iceman,lion,wolf;
    cout<<"请输入测试用例数：";
    cin>>test_num;
    for(i=0;i!=test_num;i++){
        cout<<"=============================================================================="
            <<endl<<"请分别输入第 "<<i+1<<" 组的司令部初始生命值，城市数，忠诚度减少量和结束时间"<<endl;
        config_map.push_back(config_list);
        cin>>headquarter_element>>city_num>>royalty_decrement>>deadline;
        config_map[i].push_back(headquarter_element);
        config_map[i].push_back(city_num);
        config_map[i].push_back(royalty_decrement);
        config_map[i].push_back(deadline);
        cout<<"请分别输入dragon ninja iceman lion wolf的初始生命值"<<endl;
        element_map.push_back(element_list);
        cin>>dragon>>ninja>>iceman>>lion>>wolf;
        element_map[i].push_back(dragon);
        element_map[i].push_back(ninja);
        element_map[i].push_back(iceman);
        element_map[i].push_back(lion);
        element_map[i].push_back(wolf);
        cout<<"请分别输入dragon ninja iceman lion wolf的攻击力"<<endl;
        strength_map.push_back(strength_list);
        cin>>dragon>>ninja>>iceman>>lion>>wolf;
        strength_map[i].push_back(dragon);
        strength_map[i].push_back(ninja);
        strength_map[i].push_back(iceman);
        strength_map[i].push_back(lion);
        strength_map[i].push_back(wolf);
    }
    for(i=0;i!=test_num;i++){
        cout<<"Case "<<i+1<<":"<<endl;
        shared_ptr<War> pwar = make_shared<War>(config_map[i][0],config_map[i][1],
                                                config_map[i][2],config_map[i][3],
                                                make_shared<vector<size_t>>(element_map[i]),
                                                make_shared<vector<size_t>>(strength_map[i]));
        pwar->make_war();
    }
    cout<<",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,"<<endl;
    return 0;
}


