#include "cplayermgr.h"
#include "player.h"

using namespace std;

//extern std::unordered_map<string, player*> PlayerData;

cplayermgr::cplayermgr()
{
    PlayerData.clear();
}

cplayermgr::~cplayermgr()
{
    for(auto iter = PlayerData.begin();iter != PlayerData.end();iter++)
    {
        player * temp = iter->second;
        delete temp;
        iter->second = NULL;
    }
    PlayerData.clear();
}

bool cplayermgr::find_player(string s)
{
    if(PlayerData.find(s) != PlayerData.end())
    {
        return true;
    }
    return false;
}

void cplayermgr::insert_player(string s, player* oPlayer)
{
    PlayerData.insert(make_pair(s,oPlayer));
}

player * cplayermgr::get_player(string s)
{
    if(s.size() == 0)
    {
        return NULL;
    }
    if(PlayerData.find(s) == PlayerData.end())
    {
        return NULL;
    }
    return PlayerData[s];
}