#ifndef PLAYER
#define PLAYER

#include <iostream>
#include "data.h"

using namespace std;

class player
{
private:
    string account;
    string password;
    string name;
    RoleType role;
    uint32_t room_id;
    uint8_t image_id;
    PlayerState state;

public:
    player(string acc, string pw);
	void set_account(string acc);
	string get_account();
    void set_password(string pw);
    string get_password();
    void set_name(string _name);
    string get_name();
    void set_role(RoleType _role);
    RoleType get_role();
    void set_room_id(uint32_t _room);
    uint32_t get_room_id();
    void set_image_id(uint8_t _image);
    uint8_t get_image_id();
    void set_state(PlayerState _state);
    PlayerState get_state();
};

#endif