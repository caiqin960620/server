#ifndef CPLAYERMGR
#define CPLAYERMGR

#include <iostream>
#include <unordered_map>
#include "singleton.h"

using namespace std;

class player;

class cplayermgr : public CSingleton<cplayermgr>
{
private:
    std::unordered_map<string, player*> PlayerData;
public:
    cplayermgr();
    ~cplayermgr();
    bool find_player(string s);
    void insert_player(string s, player* oPlayer);
    player* get_player(string s);
    void test()
    {
        for(auto iter = PlayerData.begin();iter != PlayerData.end();iter++)
        {
            player * temp = iter->second;
            delete temp;
            iter->second = NULL;
            //PlayerData.clear();
        }
    }
};

#endif