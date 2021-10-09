#include "cmd.h"
#include "player.h"
#include "data.h"
#include "message.h"
#include "stdio.h"
#include "cplayermgr.h"

using namespace std;

const int MAX_BUFFER = 4096;
extern std::unordered_multimap<int, player*> Fd_Player_Login;
extern pthread_mutex_t Fd_Player_Lock;

CSMsg::CSMsg()
{

}

CSMsg::CSMsg(int _fd, int _epoll_fd)
{
	fd = _fd;
	epoll_fd = _epoll_fd;
	message = "";
}

CSMsg::~CSMsg()
{
	struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.ptr = (void*)this;
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
	//cplayermgr::Inst().test();
    close(fd);
}

void CSMsg::setFd(int _fd)
{
	fd = _fd;
}

int CSMsg::getFd()
{
	return fd;
}

int CSMsg::handleRequest()
{
	perror("csmsg handle");
	uint8_t buff[MAX_BUFFER];
	bool iserror = false;

    /*开始读取*/
    int read_num = readn(fd, buff, MAX_BUFFER);
    /*读取出错则直接退出*/
	if (read_num != MSG_REQ_LOGIN_LEN && read_num != MSG_REQ_MATCH_LEN && read_num != MSG_REQ_CHANGENAME_LEN && read_num != MSG_REQ_CHANGEHEAD_LEN && read_num != MSG_REQ_LOGOUT_LEN && read_num != MSG_REQ_CANCELMATCH_LEN)
    {
        perror("DATA READ NUM IS ERROR");
		iserror = true;
		pthread_mutex_lock(&Fd_Player_Lock);
		while(Fd_Player_Login.find(fd) != Fd_Player_Login.end())
		{
			auto iter = Fd_Player_Login.find(fd);
			player* oPlayer = iter->second;
			if(oPlayer == NULL)
			{
				perror("PLAYER IS NULL");
				Fd_Player_Login.erase(iter);
				continue;
			}
			oPlayer->set_state(OUTLINE);
			Fd_Player_Login.erase(iter);
		}
		pthread_mutex_unlock(&Fd_Player_Lock);
        delete this;   //如果长度不正确，可能是关闭连接close请求，直接delete，防止服务器没有清掉fd
        return -1;
    }
    /*读取到字符串now_read 再传递到content里面*/
	message.clear();
    string now_read(buff, buff + read_num);
    message += now_read;

if(iserror == false)
{
#if CRCCHECK
    if(-1 != CheckCRC(buff, read_num))
	{
#endif
        switch((EClientServerCmds)buff[2])
		{
			case CS_REQ_LOGIN:
			{
				CSReqLogin stReq;
				if(0 != DecodeLogin(buff, MSG_REQ_LOGIN_LEN, stReq))
				{
					perror("DECODE LOGIN ERROR");
					iserror = true;
					break;
					//delete this;
					//return -1;
				}
				uint8_t * data = new uint8_t[MSG_RES_LOGIN_LEN];
				if(0 != HandleCSReqLogin(fd, data, MSG_RES_LOGIN_LEN, stReq))
				{
					perror("HANDLE LOGIN ERROR");
					iserror = true;
					//delete []data;
					//delete this;
					//return -1;
				}
				else
				{
					perror("HANDLE LOGIN SUCCESS");
				}
				delete []data;
				break;
			}
			case CS_REQ_MATCH:
			{
				CSReqMatch stReq;
				if(0 != DecodeMatch(buff, MSG_REQ_MATCH_LEN, stReq))
				{
					perror("DECODE MATCH ERROR");
					iserror = true;
					break;
					//delete this;
					//return -1;
				}
				uint8_t * data = new uint8_t[MSG_RES_MATCH_LEN];
				if(0 != HandleCSReqMatch(fd, data, MSG_RES_MATCH_LEN, stReq))
				{
					perror("HANDLE MATCH ERROR");
					iserror = true;
					//delete []data;
					//delete this;
					//return -1;
				}
				else
				{
					perror("HANDLE MATCH SUCCESS");
				}
				delete []data;
				break;
			}
			case CS_REQ_CHANGE_NAME:
			{
				CSReqChangeName stReq;
				if(0 != DecodeChangeName(buff, MSG_REQ_CHANGENAME_LEN, stReq))
				{
					perror("DECODE CHANGE NAME ERROR");
					iserror = true;
					break;
					//delete this;
					//return -1;
				}
				uint8_t * data = new uint8_t[MSG_RES_CHANGENAME_LEN];
				if(0 != HandleCSReqChangeName(fd, data, MSG_RES_CHANGENAME_LEN, stReq))
				{
					perror("HANDLE CHANGE NAME ERROR");
					iserror = true;
					//delete []data;
					//delete this;
					//return -1;
				}
				else
				{
					perror("HANDLE CHANGE NAME SUCCESS");
				}
				delete []data;
				break;
			}
			case CS_REQ_CHANGE_HEAD:
			{
				CSReqChangeHead stReq;
				if(0 != DecodeChangeHead(buff, MSG_REQ_CHANGEHEAD_LEN, stReq))
				{
					perror("DECODE CHANGE HEAD ERROR");
					iserror = true;
					break;
					//delete this;
					//return -1;
				}
				uint8_t * data = new uint8_t[MSG_RES_CHANGEHEAD_LEN];
				if(0 != HandleCSReqChangeHead(fd, data, MSG_RES_CHANGEHEAD_LEN, stReq))
				{
					perror("HANDLE CHANGE HEAD ERROR");
					iserror = true;
					//delete []data;
					//delete this;
					//return -1;
				}
				else
				{
					perror("HANDLE CHANGE HEAD SUCCESS");
				}
				delete []data;
				break;
			}
			case CS_REQ_LOGOUT:
			{
				CSReqLogout stReq;
				if(0 != DecodeLogout(buff, MSG_REQ_LOGOUT_LEN, stReq))
				{
					perror("DECODE LOGOUT ERROR");
					iserror = true;
					break;
					//delete this;
					//return -1;
				}
				uint8_t * data = new uint8_t[MSG_RES_LOGOUT_LEN];
				if(0 != HandleCSReqLogout(fd, data, MSG_RES_LOGOUT_LEN, stReq))
				{
					perror("DECODE LOGOUT ERROR");
					iserror = true;
					//return -1;
				}
				else
				{
					perror("HANDLE LOGOUT SUCCESS");
				}
				delete []data;
				usleep(100000); //防止发送过程中断开连接
				delete this;
				return 0;
			}
			case CS_REQ_CANCELMATCH:
			{
				CSReqCancelMatch stReq;
				if(0 != DecodeCancelMatch(buff, MSG_REQ_CANCELMATCH_LEN, stReq))
				{
					perror("DECODE CANCEL MATCH ERROR");
					iserror = true;
					break;
					//delete this;
					//return -1;
				}
				uint8_t * data = new uint8_t[MSG_RES_CANCELMATCH_LEN];
				if(0 != HandleCSReqCancelMatch(fd, data, MSG_RES_CANCELMATCH_LEN, stReq))
				{
					perror("HANDLE CANCEL MATCH ERROR");
					iserror = true;
					//delete []data;
					//delete this;
					//return -1;
				}
				else
				{
					perror("HANDLE CANCEL MATCH SUCCESS");
				}
				delete []data;
				break;
			}
			default:
			{
				perror("CMD ERROR");
				iserror = true;
				break;
			}
		}
#if CRCCHECK
	}
	else
	{
        perror("CRC CHECK ERROR");
		iserror = true;
		//delete this;
		//return -1;
	}
#endif
}
	__uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
    int ret = epoll_mod(epoll_fd, fd, static_cast<void*>(this), _epo_event);
    if (ret < 0)
    {
        delete this;
        return -1;
    }
	if(iserror == true)
	{
		perror("csmsg handle error");
		return -1;
	}
	cout<<"csmsg handle success";
	return 0;
}