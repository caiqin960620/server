#ifndef CMD
#define CMD

#include <iostream>

enum EClientServerCmds
{
    CS_REQ_LOGIN = 0,
    CS_RES_LOGIN = 1,
    CS_REQ_MATCH = 2,
    CS_RES_MATCH = 3,
    CS_REQ_CHANGE_NAME = 4,
    CS_RES_CHANGE_NAME = 5,
    CS_REQ_CHANGE_HEAD = 6,
    CS_RES_CHANGE_HEAD = 7,
    CS_REQ_LOGOUT = 8,
    CS_RES_LOGOUT = 9,
    CS_REQ_CANCELMATCH = 10,
    CS_RES_CANCELMATCH = 11
};

#endif