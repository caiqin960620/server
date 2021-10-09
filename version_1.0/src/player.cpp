#include "player.h"

using namespace std;

player::player(string acc, string pw)
{
    account = acc;
    password = pw;
    name = acc;
    role = NONE;
    room_id = -1;
    state = ONLINE;
}

void player::set_account(string acc)
{
    account = acc;
}

string player::get_account()
{
    return account;
}

void player::set_password(string pw)
{
    password = pw;
}

string player::get_password()
{
    return password;
}

void player::set_name(string _name)
{
    name = _name;
}

string player::get_name()
{
    return name;
}

void player::set_role(RoleType _role)
{
    role = _role;
}

RoleType player::get_role()
{
    return role;
}

void player::set_room_id(uint32_t _room)
{
    room_id = _room;
}

uint32_t player::get_room_id()
{
    return room_id;
}

void player::set_image_id(uint8_t _image)
{
    image_id = _image;
}

uint8_t player::get_image_id()
{
    return image_id;
}

void player::set_state(PlayerState _state)
{
    state = _state;
}

PlayerState player::get_state()
{
    return state;
}