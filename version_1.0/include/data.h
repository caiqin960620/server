#ifndef DATA
#define DATA

#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>
#include "common.h"
#include "cmd.h"
#include "util.h"
#include "epoll.h"
#include "requestData.h"

using namespace std;

#define CRCCHECK 1

#define ACCOUNT_MAX_LEN 10
#define NAME_MAX_LEN 10
#define PASSWORD_MAX_LEN 10

#define MSG_REQ_LOGIN_LEN  31
#define MSG_REQ_MATCH_LEN  22
#define MSG_REQ_CHANGENAME_LEN  31
#define MSG_REQ_CHANGEHEAD_LEN  22
#define MSG_REQ_LOGOUT_LEN  21
#define MSG_REQ_CANCELMATCH_LEN  21

#define MSG_RES_LOGIN_LEN  33
#define MSG_RES_MATCH_LEN  27
#define MSG_RES_CHANGENAME_LEN  22
#define MSG_RES_CHANGEHEAD_LEN  22
#define MSG_RES_LOGOUT_LEN  22
#define MSG_RES_CANCELMATCH_LEN  22

#define LOGIN_OFFSET  5
#define MATCH_OFFSET  5
#define CHANGENAME_OFFSET  5
#define CHANGEHEAD_OFFSET  5
#define LOGOUT_OFFSET  5
#define CANCELMATCH_OFFSET  5

#define HEAD_BYTE1 0xEB
#define HEAD_BYTE2 0x90
#define END_BYTE1 0xEB
#define END_BYTE2 0x90

#define RANDOM_NUM 10

class CSMsg;
class player;

enum RoleType
{
	NONE = 0,
	BAG = 1,
	CHILD = 2
};

enum PlayerState
{
	OUTLINE = 0,
	ONLINE = 1,
	MATCHING = 2,
	GAME = 3
};

typedef struct
{
	string Player_Account;
	string Player_Password;
}CSReqLogin;

typedef struct
{
	string Player_Account;
	uint8_t Player_Role;
}CSReqMatch;

typedef struct
{
	string Player_Account;
	string Player_ReName;
}CSReqChangeName;

typedef struct
{
	string Player_Account;
	uint8_t Player_ReImageId;
}CSReqChangeHead;

typedef struct
{
	string Player_Account;
}CSReqLogout;

typedef struct
{
	string Player_Account;
}CSReqCancelMatch;

typedef struct 
{
	uint8_t result;
	string Player_Account;
	string Player_Name;
	uint8_t Player_Image_Id;
}CSResLogin;

typedef struct
{
	uint8_t result;
	string Player_Account;
	uint8_t Player_Role;
	uint32_t Room_Id;
}CSResMatch;

typedef struct
{
	uint8_t result;
	string Player_Account;
}CSResChangeName;

typedef struct
{
	uint8_t result;
	string Player_Account;
}CSResChangeHead;

typedef struct
{
	uint8_t result;
	string Player_Account;
}CSResLogout;

typedef struct
{
	uint8_t result;
	string Player_Account;
}CSResCancelMatch;

int CheckCRC(uint8_t * data, int datalen);
int CalculateCRC(uint8_t * data, int datalen);
int ProccessMatch();
int NotifyMatch(int fd);
int NotifyMatchFail(int fd);

int DecodeLogin(uint8_t * data, int datalen, CSReqLogin& stReq);
int DecodeMatch(uint8_t * data, int datalen, CSReqMatch& stReq);
int DecodeChangeName(uint8_t * data, int datalen, CSReqChangeName& stReq);
int DecodeChangeHead(uint8_t * data, int datalen, CSReqChangeHead& stReq);
int DecodeLogout(uint8_t * data, int datalen, CSReqLogout& stReq);
int DecodeCancelMatch(uint8_t * data, int datalen, CSReqCancelMatch& stReq);

int EncodeData(uint8_t * data, int datalen);
int EncodeLogin(uint8_t * data, int datalen, CSResLogin& stRes);
int EncodeMatch(uint8_t * data, int datalen, CSResMatch& stRes);
int EncodeChangeName(uint8_t * data, int datalen, CSResChangeName& stRes);
int EncodeChangeHead(uint8_t * data, int datalen, CSResChangeHead& stRes);
int EncodeLogout(uint8_t * data, int datalen, CSResLogout& stRes);
int EncodeCancelMatch(uint8_t * data, int datalen, CSResCancelMatch& stRes);

int HandleCSReqLogin(int fd, uint8_t * data, int datalen, CSReqLogin& stReq); 
int HandleCSReqMatch(int fd, uint8_t * data, int datalen, CSReqMatch& stReq);
int HandleCSReqChangeName(int fd, uint8_t * data, int datalen, CSReqChangeName& stReq);
int HandleCSReqChangeHead(int fd, uint8_t * data, int datalen, CSReqChangeHead& stReq);
int HandleCSReqLogout(int fd, uint8_t * data, int datalen, CSReqLogout& stReq); 
int HandleCSReqCancelMatch(int fd, uint8_t * data, int datalen, CSReqCancelMatch& stReq); 

#endif