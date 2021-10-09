#include "data.h" 
#include "player.h"
#include "cplayermgr.h"
#include <deque>

using namespace std;

//pthread_mutex_t Bag_Player_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PlayerData_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Fd_Player_Lock = PTHREAD_MUTEX_INITIALIZER;

std::deque<int> Bag_Player;
std::deque<int> Child_Player;
//std::unordered_map<string, player*> PlayerData;
std::unordered_multimap<int, player*> Fd_Player_Login;
std::unordered_multimap<int, player*> Fd_Player;
std::unordered_multimap<int, player*> Fd_Player_Record;   // 改为双端队列，加一个stack（没有clear)，注意清空和加锁
std::deque<int> Bag_Player_Record;
std::deque<int> Child_Player_Record;  //注意清空和加锁

uint32_t room_count = 0;
uint8_t room_bag_num = 0;
uint8_t room_child_num = 0;  //注意初始化为0和加锁

int CheckCRC(uint8_t * data, int datalen)
{
    uint8_t * pData = data + 3;
    if(u8u16(pData) != datalen)
    {
        /*int a = u8u16(data);
        cout<<a<<endl;
        for(int i=0;i<datalen;i++)
        {
            int b = data[i];
            cout<<b<<endl;
        }*/
        perror("DATALEN ERROR");
        return -1;
    }
    if(*data == HEAD_BYTE1 && *(data + 1) == HEAD_BYTE2 && *(data + datalen - 2) == END_BYTE1 && *(data + datalen - 1) == END_BYTE2)
    {
        uint32_t CheckSum = 0;
		uint8_t *pData = &data[2];
		for (int i = 0; i < datalen - 8; i++)
		{
			CheckSum += *pData++;
		}
		uint32_t Crc = u8u32(pData);

		if (CheckSum == Crc)
		{
			return 0;
		}
		else
		{
            perror("CRC ERROR");
			return -1;
		}
    }
    else
    {
        perror("HEAD OR END ERROR");
        return -1;
    }
}

int CalculateCRC(uint8_t * data, int datalen)
{
    uint32_t CheckSum = 0;
    for (int i = 0; i < datalen; i++)
    {
        CheckSum += *data++;
    }
    return CheckSum;
}

int DecodeLogin(uint8_t * data, int datalen, CSReqLogin& stReq)
{
    if(datalen != MSG_REQ_LOGIN_LEN)
    {
        perror("LOGIN DATALEN ERROR");
        return -1;
    }
    uint8_t * pData = data + LOGIN_OFFSET;
    string Account(pData, pData + ACCOUNT_MAX_LEN);
    stReq.Player_Account = Account;
    pData += ACCOUNT_MAX_LEN;
    string Password(pData, pData + PASSWORD_MAX_LEN);
    stReq.Player_Password = Password;
    return 0;
}

int DecodeMatch(uint8_t * data, int datalen, CSReqMatch& stReq)
{
    if(datalen != MSG_REQ_MATCH_LEN)
    {
        perror("MATCH DATALEN ERROR");
        return -1;
    }
    uint8_t * pData = data + MATCH_OFFSET;
    string Account(pData, pData + ACCOUNT_MAX_LEN);
    stReq.Player_Account = Account;
    pData += ACCOUNT_MAX_LEN;
    stReq.Player_Role = *pData;
    return 0;
}

int DecodeChangeName(uint8_t * data, int datalen, CSReqChangeName& stReq)
{
    if(datalen != MSG_REQ_CHANGENAME_LEN)
    {
        perror("CHANGENAME DATALEN ERROR");
        return -1;
    }
    uint8_t * pData = data + CHANGENAME_OFFSET;
    string Account(pData, pData + ACCOUNT_MAX_LEN);
    stReq.Player_Account = Account;
    pData += ACCOUNT_MAX_LEN;
    string ReName(pData, pData + NAME_MAX_LEN);
    stReq.Player_ReName = ReName;
    return 0;
}

int DecodeChangeHead(uint8_t * data, int datalen, CSReqChangeHead& stReq)
{
    if(datalen != MSG_REQ_CHANGEHEAD_LEN)
    {
        perror("CHANGEHEAD DATALEN ERROR");
        return -1;
    }
    uint8_t * pData = data + CHANGEHEAD_OFFSET;
    string Account(pData, pData + ACCOUNT_MAX_LEN);
    stReq.Player_Account = Account;
    pData += ACCOUNT_MAX_LEN;
    stReq.Player_ReImageId = *pData;
    return 0;
}

int DecodeLogout(uint8_t * data, int datalen, CSReqLogout& stReq)
{
    if(datalen != MSG_REQ_LOGOUT_LEN)
    {
        perror("LOGOUT DATALEN ERROR");
        return -1;
    }
    uint8_t * pData = data + LOGOUT_OFFSET;
    string Account(pData, pData + ACCOUNT_MAX_LEN);
    stReq.Player_Account = Account;
    return 0;
}

int DecodeCancelMatch(uint8_t * data, int datalen, CSReqCancelMatch& stReq)
{
    if(datalen != MSG_REQ_CANCELMATCH_LEN)
    {
        perror("CANCELMATCH DATALEN ERROR");
        return -1;
    }
    uint8_t * pData = data + CANCELMATCH_OFFSET;
    string Account(pData, pData + ACCOUNT_MAX_LEN);
    stReq.Player_Account = Account;
    return 0;
}

int EncodeData(uint8_t * data, int datalen)
{
    data[0] = HEAD_BYTE1;
    data[1] = HEAD_BYTE2;
    data[datalen - 2] = END_BYTE1;
    data[datalen - 1] = END_BYTE2;
    uint8_t * pData = data + 2;
    uint32_t CheckSum = CalculateCRC(pData, datalen - 8);
    uint8_t * temp = data + datalen - 6;
    if(0 != CheckSum)
    {
        u8u32(temp) = CheckSum;
        return 0;
    }
    else
    {
        u8u32(temp) = 0;
        perror("CALCULATE CRC ERROR");
        return -1;
    }
}

int EncodeLogin(uint8_t * data, int datalen, CSResLogin& stRes)
{
    memset(data, 0, MSG_RES_LOGIN_LEN);
    char * pData = (char*)data;
    EClientServerCmds cmd_id = CS_RES_LOGIN;
    pData[2] = (uint8_t)cmd_id;
    char * temp = pData + 3;
    u8u16(temp) = datalen;
    pData[5] = stRes.result;
    pData += 6;
    strncpy(pData, stRes.Player_Account.c_str(), stRes.Player_Account.length());
    pData += ACCOUNT_MAX_LEN;
    strncpy(pData, stRes.Player_Name.c_str(), stRes.Player_Name.length());
    pData += NAME_MAX_LEN;
    *pData = stRes.Player_Image_Id;

    if(0 != EncodeData(data, datalen))
    {
        perror("ENCODE ERROR");
        return -1;
    }
    return 0;
}

int EncodeMatch(uint8_t * data, int datalen, CSResMatch& stRes)
{
    memset(data, 0, MSG_RES_MATCH_LEN);
    char * pData = (char*)data;
    EClientServerCmds cmd_id = CS_RES_MATCH;
    pData[2] = (uint8_t)cmd_id;
    char * temp = pData + 3;
    u8u16(temp) = datalen;
    pData[5] = stRes.result;
    pData += 6;
    strncpy(pData, stRes.Player_Account.c_str(), stRes.Player_Account.length());
    pData += ACCOUNT_MAX_LEN;
    *pData = stRes.Player_Role;
    pData++;
    u8u32(pData) = stRes.Room_Id;

    if(0 != EncodeData(data, datalen))
    {
        perror("ENCODE ERROR");
        return -1;
    }
    return 0;
}

int EncodeChangeName(uint8_t * data, int datalen, CSResChangeName& stRes)
{
    memset(data, 0, MSG_RES_CHANGENAME_LEN);
    char * pData = (char*)data;
    EClientServerCmds cmd_id = CS_RES_CHANGE_NAME;
    pData[2] = (uint8_t)cmd_id;
    char * temp = pData + 3;
    u8u16(temp) = datalen;
    pData[5] = stRes.result;
    pData += 6;
    strncpy(pData, stRes.Player_Account.c_str(), stRes.Player_Account.length());

    if(0 != EncodeData(data, datalen))
    {
        perror("ENCODE ERROR");
        return -1;
    }
    return 0;
}

int EncodeChangeHead(uint8_t * data, int datalen, CSResChangeHead& stRes)
{
    memset(data, 0, MSG_RES_CHANGEHEAD_LEN);
    char * pData = (char*)data;
    EClientServerCmds cmd_id = CS_RES_CHANGE_HEAD;
    pData[2] = (uint8_t)cmd_id;
    char * temp = pData + 3;
    u8u16(temp) = datalen;
    pData[5] = stRes.result;
    pData += 6;
    strncpy(pData, stRes.Player_Account.c_str(), stRes.Player_Account.length());

    if(0 != EncodeData(data, datalen))
    {
        perror("ENCODE ERROR");
        return -1;
    }
    return 0;
}

int EncodeLogout(uint8_t * data, int datalen, CSResLogout& stRes)
{
    memset(data, 0, MSG_RES_LOGOUT_LEN);
    char * pData = (char*)data;
    EClientServerCmds cmd_id = CS_RES_LOGOUT;
    pData[2] = (uint8_t)cmd_id;
    char * temp = pData + 3;
    u8u16(temp) = datalen;
    pData[5] = stRes.result;
    pData += 6;
    strncpy(pData, stRes.Player_Account.c_str(), stRes.Player_Account.length());

    if(0 != EncodeData(data, datalen))
    {
        perror("ENCODE ERROR");
        return -1;
    }
    return 0;
}

int EncodeCancelMatch(uint8_t * data, int datalen, CSResCancelMatch& stRes)
{
    memset(data, 0, MSG_RES_CANCELMATCH_LEN);
    char * pData = (char*)data;
    EClientServerCmds cmd_id = CS_RES_CANCELMATCH;
    pData[2] = (uint8_t)cmd_id;
    char * temp = pData + 3;
    u8u16(temp) = datalen;
    pData[5] = stRes.result;
    pData += 6;
    strncpy(pData, stRes.Player_Account.c_str(), stRes.Player_Account.length());

    if(0 != EncodeData(data, datalen))
    {
        perror("ENCODE ERROR");
        return -1;
    }
    return 0;
}

int HandleCSReqLogin(int fd, uint8_t * data, int datalen, CSReqLogin& stReq)
{
    if(stReq.Player_Account.length() == 0 || stReq.Player_Password.length() == 0)
    {
        perror("ACCOUNT LENGTH ERROR");
        return -1;
    }
    CSResLogin stRes;
    pthread_mutex_lock(&PlayerData_Lock);
    //for(int i=0;i<stReq.Player_Account.size();i++)
    //{
    //    int temp = stReq.Player_Account[i];
    //    cout<<temp;
    //}
    //if(PlayerData.find(stReq.Player_Account) != PlayerData.end())
    if(cplayermgr::Inst().find_player(stReq.Player_Account) && Fd_Player_Login.find(fd) == Fd_Player_Login.end())
    {
        //player * oPlayer = PlayerData[stReq.Player_Account];
        player * oPlayer = cplayermgr::Inst().get_player(stReq.Player_Account);
        if(oPlayer == NULL)
        {
            perror("PLAYER IS NULL");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        PlayerState mState = oPlayer->get_state();
        //if(mState == ONLINE || mState == MATCHING || mState == GAME)   // 下线设为outline
        if(mState != OUTLINE)
        {
            perror("PLAYER IS ONLINE");
            stRes.result = -3;
            //pthread_mutex_unlock(&PlayerData_Lock);
            //return -1;
        }
        else
        {
            pthread_mutex_lock(&Fd_Player_Lock);
            Fd_Player_Login.insert(make_pair(fd, oPlayer));
            pthread_mutex_unlock(&Fd_Player_Lock);
            if(oPlayer->get_password() != stReq.Player_Password)
            {
                perror("PASSWORD IS ERROR");
                stRes.result = -1;
            }
            else
            {
                stRes.result = 1;
                stRes.Player_Account = stReq.Player_Account;
                stRes.Player_Name = oPlayer->get_name();
                stRes.Player_Image_Id = oPlayer->get_image_id();
                oPlayer->set_state(ONLINE);
            }
        }
    }
    else if(!cplayermgr::Inst().find_player(stReq.Player_Account) && Fd_Player_Login.find(fd) == Fd_Player_Login.end())
    {
        player * oPlayer = new player(stReq.Player_Account, stReq.Player_Password);
        //PlayerData.insert(make_pair(stReq.Player_Account, oPlayer));
        cplayermgr::Inst().insert_player(stReq.Player_Account, oPlayer);
        uint8_t Image_Id = random() % RANDOM_NUM + 1;
        oPlayer->set_image_id(Image_Id);
        pthread_mutex_lock(&Fd_Player_Lock);
        Fd_Player_Login.insert(make_pair(fd, oPlayer));
        pthread_mutex_unlock(&Fd_Player_Lock);
        stRes.result = 2;
        stRes.Player_Account = stReq.Player_Account;
        stRes.Player_Name = stReq.Player_Account;
        stRes.Player_Image_Id = oPlayer->get_image_id();
    }
    else
    {
        perror("ONLY ONE ACCOUNT TO LOGIN");
        stRes.result = -2;
    }
    pthread_mutex_unlock(&PlayerData_Lock);
    if(0 != EncodeLogin(data, datalen, stRes))
    {
        perror("ENCODE LOGIN ERROR");
        return -1;
    }
    size_t send_len = writen(fd, data, MSG_RES_LOGIN_LEN);
	if(send_len != MSG_RES_LOGIN_LEN)
    {
        perror("Send file failed");
        return -1;
    }
    return 0;
}

int HandleCSReqChangeName(int fd, uint8_t * data, int datalen, CSReqChangeName& stReq)
{
    if(stReq.Player_Account.length() == 0 || stReq.Player_ReName.length() == 0)
    {
        perror("ACCOUNT LENGTH ERROR");
        //
        return -1;
    }
    CSResChangeName stRes;
    pthread_mutex_lock(&PlayerData_Lock);
    //if(PlayerData.find(stReq.Player_Account) != PlayerData.end())
    if(cplayermgr::Inst().find_player(stReq.Player_Account))
    {
        //player * oPlayer = PlayerData[stReq.Player_Account];
        player * oPlayer = cplayermgr::Inst().get_player(stReq.Player_Account);
        if(oPlayer == NULL)
        {
            perror("PLAYER IS NULL");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        oPlayer->set_name(stReq.Player_ReName);
        stRes.result = 0;
        stRes.Player_Account = stReq.Player_Account;
    }
    else
    {
        perror("PLAYER IS NOT EXIST");
        stRes.result = -1;
    }
    pthread_mutex_unlock(&PlayerData_Lock);
    if(0 != EncodeChangeName(data, datalen, stRes))
    {
        perror("ENCODE CHANGE NAME ERROR");
        return -1;
    }
    size_t send_len = writen(fd, data, MSG_RES_CHANGENAME_LEN);
	if(send_len != MSG_RES_CHANGENAME_LEN)
    {
        perror("Send file failed");
        return -1;
    }
    return 0;
}

int HandleCSReqChangeHead(int fd, uint8_t * data, int datalen, CSReqChangeHead& stReq)
{
    if(stReq.Player_Account.length() == 0)
    {
        perror("ACCOUNT LENGTH ERROR");
        return -1;
    }
    if(stReq.Player_ReImageId > RANDOM_NUM || stReq.Player_ReImageId < 1)
    {
        perror("IMAGE ID ERROR");
        //
        return -1;
    }
    CSResChangeHead stRes;
    pthread_mutex_lock(&PlayerData_Lock);
    //if(PlayerData.find(stReq.Player_Account) != PlayerData.end())
    if(cplayermgr::Inst().find_player(stReq.Player_Account))
    {
        //player * oPlayer = PlayerData[stReq.Player_Account];
        player * oPlayer = cplayermgr::Inst().get_player(stReq.Player_Account);
        if(oPlayer == NULL)
        {
            perror("PLAYER IS NULL");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        oPlayer->set_image_id(stReq.Player_ReImageId);
        stRes.result = 0;
        stRes.Player_Account = stReq.Player_Account;
    }
    else
    {
        perror("PLAYER IS NOT EXIST");
        stRes.result = -1;
    }
    pthread_mutex_unlock(&PlayerData_Lock);
    if(0 != EncodeChangeHead(data, datalen, stRes))
    {
        perror("ENCODE CHANGE HEAD ERROR");
        return -1;
    }
    size_t send_len = writen(fd, data, MSG_RES_CHANGEHEAD_LEN);
	if(send_len != MSG_RES_CHANGEHEAD_LEN)
    {
        perror("Send file failed");
        return -1;
    }
    return 0;
}

int HandleCSReqLogout(int fd, uint8_t * data, int datalen, CSReqLogout& stReq)
{
    if(stReq.Player_Account.length() == 0)
    {
        perror("ACCOUNT LENGTH ERROR");
        return -1;
    }
    CSResLogout stRes;
    pthread_mutex_lock(&PlayerData_Lock);
    //if(PlayerData.find(stReq.Player_Account) != PlayerData.end())
    if(cplayermgr::Inst().find_player(stReq.Player_Account))
    {
        //player * oPlayer = PlayerData[stReq.Player_Account];
        player * oPlayer = cplayermgr::Inst().get_player(stReq.Player_Account);
        if(oPlayer == NULL)
        {
            perror("PLAYER IS NULL");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        oPlayer->set_state(OUTLINE);
        pthread_mutex_lock(&Fd_Player_Lock);
		while(Fd_Player_Login.find(fd) != Fd_Player_Login.end())
		{
			auto iter = Fd_Player_Login.find(fd);
			player* temp_player = iter->second;
			if(temp_player == NULL)
			{
				perror("PLAYER IS NULL");
				Fd_Player_Login.erase(iter);
				continue;
			}
            temp_player->set_state(OUTLINE);
			Fd_Player_Login.erase(iter);
		}
		pthread_mutex_unlock(&Fd_Player_Lock);
        stRes.result = 0;
        stRes.Player_Account = stReq.Player_Account;
    }
    else
    {
        perror("PLAYER IS NOT EXIST");
        stRes.result = -1;
    }
    pthread_mutex_unlock(&PlayerData_Lock);
    if(0 != EncodeLogout(data, datalen, stRes))
    {
        perror("ENCODE LOGOUT ERROR");
        return -1;
    }
    size_t send_len = writen(fd, data, MSG_RES_LOGOUT_LEN);
	if(send_len != MSG_RES_LOGOUT_LEN)
    {
        perror("Send file failed");
        return -1;
    }
    //close(fd);  //delete
    return 0;
}

int HandleCSReqCancelMatch(int fd, uint8_t * data, int datalen, CSReqCancelMatch& stReq)
{
    if(stReq.Player_Account.length() == 0)
    {
        perror("ACCOUNT LENGTH ERROR");
        return -1;
    }
    CSResCancelMatch stRes;
    pthread_mutex_lock(&PlayerData_Lock); //要用到两个锁
    if(cplayermgr::Inst().find_player(stReq.Player_Account))
    {
        player * oPlayer = cplayermgr::Inst().get_player(stReq.Player_Account);
        if(oPlayer == NULL)
        {
            perror("PLAYER IS NULL");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        PlayerState mState = oPlayer->get_state();  //
        if(mState != MATCHING)
        {
            perror("PLAYER STATE ERROR");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        RoleType role = oPlayer->get_role();
        bool cancel_flag = false;
        std::deque<int> temp_que;
        if(role == BAG && !Bag_Player.empty())
        {
            while(!Bag_Player.empty())
            {
                if(Bag_Player.front() == fd)
                {
                    cancel_flag = true;
                    Bag_Player.pop_front();
                    break;
                }
                temp_que.push_back(Bag_Player.front());
                Bag_Player.pop_front();
            }
            while(!temp_que.empty())
            {
                Bag_Player.push_front(temp_que.back());
                temp_que.pop_back();
            }
        }
        else if(role == CHILD && !Child_Player.empty())
        {
            while(!Child_Player.empty())
            {
                if(Child_Player.front() == fd)
                {
                    cancel_flag = true;
                    Child_Player.pop_front();
                    break;
                }
                temp_que.push_back(Child_Player.front());
                Child_Player.pop_front();
            }
            while(!temp_que.empty())
            {
                Child_Player.push_front(temp_que.back());
                temp_que.pop_back();
            }
        }
        else  //
        {
            perror("ROLE ERROR");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        //Fd_map
        pthread_mutex_lock(&Fd_Player_Lock);
        for(auto iter = Fd_Player.begin();iter != Fd_Player.end();iter++)
        {
            if(cancel_flag == false)
            {
                break;
            }
            player* temp_player = iter->second;
            if(temp_player == NULL)
            {
                perror("PLAYER IS NULL");
                continue;
            }
            if((temp_player->get_account() == oPlayer->get_account()) && iter->first == fd)
            {
                Fd_Player.erase(iter);
                break;
            }
        }
        pthread_mutex_unlock(&Fd_Player_Lock);
        if(cancel_flag == false)
        {
            perror("PLAYER IS NOT EXIST");
            stRes.result = -1;
        }
        else
        {
            oPlayer->set_state(ONLINE);
            stRes.result = 0;
            stRes.Player_Account = stReq.Player_Account;
        }
    }
    else
    {
        perror("PLAYER IS NOT EXIST");
        stRes.result = -1;
    }
    pthread_mutex_unlock(&PlayerData_Lock);
    if(0 != EncodeCancelMatch(data, datalen, stRes))
    {
        perror("ENCODE CANCEL MATCH ERROR");
        return -1;
    }
    size_t send_len = writen(fd, data, MSG_RES_CANCELMATCH_LEN);
	if(send_len != MSG_RES_CANCELMATCH_LEN)
    {
        perror("Send file failed");
        return -1;
    }
    return 0;
}

int HandleCSReqMatch(int fd, uint8_t * data, int datalen, CSReqMatch& stReq)
{
    if(stReq.Player_Account.length() == 0)
    {
        perror("ACCOUNT LENGTH ERROR");
        return -1;
    }
    pthread_mutex_lock(&PlayerData_Lock);
    //if(PlayerData.find(stReq.Player_Account) != PlayerData.end())
    if(cplayermgr::Inst().find_player(stReq.Player_Account))
    {
        //player * oPlayer = PlayerData[stReq.Player_Account];
        player * oPlayer = cplayermgr::Inst().get_player(stReq.Player_Account);
        if(oPlayer == NULL)
        {
            perror("PLAYER IS NULL");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        PlayerState mState = oPlayer->get_state();
        if(mState != ONLINE)
        {
            perror("PLAYER STATE ERROR");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        oPlayer->set_state(MATCHING);
        if((RoleType)stReq.Player_Role == BAG)
        {
            oPlayer->set_role(BAG);
            Bag_Player.push_back(fd);
            Fd_Player.insert(make_pair(fd, oPlayer));
        }
        else if((RoleType)stReq.Player_Role == CHILD)
        {
            oPlayer->set_role(CHILD);
            Child_Player.push_back(fd);
            Fd_Player.insert(make_pair(fd, oPlayer));
        }
        else
        {
            perror("ROLE ERROR");
            //
            oPlayer->set_state(ONLINE);
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
    }
    else
    {
        CSResMatch stRes;
        perror("PLAYER IS NOT EXIST");
        stRes.result = -1;
        if(0 != EncodeMatch(data, datalen, stRes))
        {
            perror("ENCODE LOGIN ERROR");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        size_t send_len = writen(fd, data, MSG_RES_MATCH_LEN);
		if(send_len != MSG_RES_MATCH_LEN)
        {
            perror("Send file failed");
            pthread_mutex_unlock(&PlayerData_Lock);
            return -1;
        }
        pthread_mutex_unlock(&PlayerData_Lock);
        return -1;
    }
    int Bag_Size = Bag_Player.size();
    int Child_Size = Child_Player.size();
    pthread_mutex_unlock(&PlayerData_Lock);

    if(Bag_Size >= 2 && Child_Size >= 4)
    {
        if(0 != ProccessMatch())
        {
            perror("PROCCESS MATCH ERROR");
            return -1;
        }
        return 0;
    }
    else
    {
        CSResMatch stRes;
        stRes.result = 0; //表明收到了，正在匹配中
        if(0 != EncodeMatch(data, datalen, stRes))
        {
            perror("ENCODE MATCH ERROR");
            return -1;
        }
        size_t send_len = writen(fd, data, MSG_RES_MATCH_LEN);
		if(send_len != MSG_RES_MATCH_LEN)
        {
            perror("Send file failed");
            return -1;
        }
        return 0;
    }
}

int ProccessMatch()
{
    int i = 0;
    pthread_mutex_lock(&PlayerData_Lock);
    room_count++;
    room_bag_num = 0;
    room_child_num = 0;
    Fd_Player_Record.clear();
    Bag_Player_Record.clear();
    Child_Player_Record.clear();
    std::deque<int> temp_queue; //最后再pop出来
    temp_queue.clear();
    while(i < 2 && !Bag_Player.empty())
    {
        int fd = Bag_Player.front();
        if(0 != NotifyMatch(fd))
        {
            bool match_flag = false;
            int temp_fd = 0;
            temp_queue.push_back(Bag_Player.front());
            if((room_bag_num + Bag_Player.size()) > 2)
            {
                Bag_Player.pop_front();
                while(!Bag_Player.empty())
                {
                    temp_fd = Bag_Player.front();
                    if(0 == NotifyMatch(temp_fd))
                    {
                        match_flag = true;
                        break;
                    }
                    temp_queue.push_back(temp_fd);
                    Bag_Player.pop_front();
                }
            }
            if(match_flag == true)
            {
                Bag_Player_Record.push_back(temp_fd);
                Bag_Player.pop_front();
                room_bag_num++;
                perror("PROCCESS BAG MATCH SUCCESS");
                continue;
            }
            else
            {
                //notifyfail
                //temp_queue pop
                //record pop
                while(!temp_queue.empty())
                {
                    //int temp = temp_queue.back();
                    //NotifyMatchFail(temp);    //是否需要
                    Bag_Player.push_front(temp_queue.back());
                    temp_queue.pop_back();
                }
                while(!Bag_Player_Record.empty())
                {
                    int temp = Bag_Player_Record.back();
                    NotifyMatchFail(temp);
                    Bag_Player.push_front(Bag_Player_Record.back());
                    Bag_Player_Record.pop_back();
                }
                perror("NOTIFY MATCH ERROR");
                pthread_mutex_unlock(&PlayerData_Lock);
                return -1;  //
            }
        }
        Bag_Player_Record.push_back(fd);  //notify失败的玩家是否push保存了，判断后面是否有其他玩家，优先让其他玩家进入游戏
        Bag_Player.pop_front();
        room_bag_num++;
        perror("PROCCESS BAG MATCH SUCCESS");
        i++;
    }
    //temp_queue pop
    while(!temp_queue.empty())
    {
        Bag_Player.push_front(temp_queue.back());
        temp_queue.pop_back();
    }
    i = 0;
    Fd_Player_Record.clear();   //需要，因为notifyfail可能返回-1，这样就不会在那个函数里面清除了
    temp_queue.clear();
    while(i < 4 && !Child_Player.empty())
    {
        int fd = Child_Player.front();
        if(0 != NotifyMatch(fd))
        {
            bool match_flag = false;
            int temp_fd = 0;
            temp_queue.push_back(Child_Player.front());
            if(room_child_num + Child_Player.size() > 4)
            {
                Child_Player.pop_front();
                while(!Child_Player.empty())
                {
                    temp_fd = Child_Player.front();
                    if(0 == NotifyMatch(temp_fd))
                    {
                        match_flag = true;
                        break;
                    }
                    temp_queue.push_back(temp_fd);
                    Child_Player.pop_front();
                }
            }
            if(match_flag == true)
            {
                Child_Player_Record.push_back(temp_fd);
                Child_Player.pop_front();
                room_child_num++;
                perror("PROCCESS CHILD MATCH SUCCESS");
                continue;
            }
            else
            {
                while(!temp_queue.empty())
                {
                    Child_Player.push_front(temp_queue.back());
                    temp_queue.pop_back();
                }
                while(!Child_Player_Record.empty())
                {
                    int temp = Child_Player_Record.back();
                    NotifyMatchFail(temp);
                    Child_Player.push_front(Child_Player_Record.back());
                    Child_Player_Record.pop_back();
                }
                perror("NOTIFY MATCH ERROR");
                pthread_mutex_unlock(&PlayerData_Lock);
                return -1; //
            }
        }
        Child_Player_Record.push_back(fd);
        Child_Player.pop_front();
        room_child_num++;
        perror("PROCCESS CHILD MATCH SUCCESS");
        i++;
    }
    while(!temp_queue.empty())
    {
        Child_Player.push_front(temp_queue.back());
        temp_queue.pop_back();
    }
    pthread_mutex_unlock(&PlayerData_Lock);
    return 0;
}

int NotifyMatch(int fd)
{
    pthread_mutex_lock(&Fd_Player_Lock);
    if(Fd_Player.find(fd) != Fd_Player.end())
    {
        auto iter = Fd_Player.find(fd);
        //player * oPlayer = Fd_Player[fd];
        player * oPlayer = iter->second;
        if(oPlayer == NULL)
        {
            perror("PLAYER IS NULL");
            pthread_mutex_unlock(&Fd_Player_Lock);
            return -1;
        }
        PlayerState mState = oPlayer->get_state();
        if(mState != MATCHING)
        {
            perror("PLAYER IS NOT MATCH");
            pthread_mutex_unlock(&Fd_Player_Lock);
            return -1;
        }
        oPlayer->set_room_id(room_count);
        uint8_t * data = new uint8_t[MSG_RES_MATCH_LEN];
        CSResMatch stRes;
        stRes.result = 1;
        stRes.Player_Account = oPlayer->get_account();
        stRes.Player_Role = oPlayer->get_role();
        stRes.Room_Id = room_count;
        if(0 != EncodeMatch(data, MSG_RES_MATCH_LEN, stRes))
        {
            perror("ENCODE MATCH ERROR");
            pthread_mutex_unlock(&Fd_Player_Lock);
            delete []data;
            return -1;
        }
	    size_t send_len = writen(fd, data, MSG_RES_MATCH_LEN);
	    if(send_len != MSG_RES_MATCH_LEN)
        {
            int resend_time = 0;
            while(resend_time < 3 && send_len != MSG_RES_MATCH_LEN)
            {
                //usleep
                send_len = writen(fd, data, MSG_RES_MATCH_LEN);
                if(send_len == MSG_RES_MATCH_LEN)
                {
                    break;
                }
                resend_time++;
            }
            if(resend_time == 3)
            {
                perror("Send file failed");
                pthread_mutex_unlock(&Fd_Player_Lock);
                delete []data;
                return -1;
            }
        }
        delete []data;
        oPlayer->set_state(GAME);
        Fd_Player_Record.insert(make_pair(fd, oPlayer));
        Fd_Player.erase(iter);
    }
    else
    {
        perror("PLAYER IS NOT EXIST");
        pthread_mutex_unlock(&Fd_Player_Lock);
        return -1;
    }
    pthread_mutex_unlock(&Fd_Player_Lock);
    return 0;
}

int NotifyMatchFail(int fd)
{
    pthread_mutex_lock(&Fd_Player_Lock);
    if(Fd_Player_Record.find(fd) != Fd_Player_Record.end())
    {
        auto iter = Fd_Player_Record.find(fd);
        player * oPlayer = iter->second;
        if(oPlayer == NULL)
        {
            perror("PLAYER IS NULL");
            pthread_mutex_unlock(&Fd_Player_Lock);
            return -1;
        }
        Fd_Player.insert(make_pair(fd, oPlayer));
        uint8_t * data = new uint8_t[MSG_RES_MATCH_LEN];
        CSResMatch stRes;
        stRes.result = -2;
        stRes.Player_Account = oPlayer->get_account();
        stRes.Player_Role = oPlayer->get_role();
        stRes.Room_Id = room_count;
        if(0 != EncodeMatch(data, MSG_RES_MATCH_LEN, stRes))
        {
            perror("ENCODE MATCH ERROR");
            pthread_mutex_unlock(&Fd_Player_Lock);
            delete []data;
            return -1;
        }
	    size_t send_len = writen(fd, data, MSG_RES_MATCH_LEN);
	    if(send_len != MSG_RES_MATCH_LEN)
        {
            int resend_time = 0;
            while(resend_time < 3 && send_len != MSG_RES_MATCH_LEN)
            {
                //usleep
                send_len = writen(fd, data, MSG_RES_MATCH_LEN);
                if(send_len == MSG_RES_MATCH_LEN)
                {
                    break;
                }
                resend_time++;
            }
            if(resend_time == 3)
            {
                perror("Send file failed");
                pthread_mutex_unlock(&Fd_Player_Lock);
                delete []data;
                return -1;
            }
        }
        delete []data;
        Fd_Player_Record.erase(iter);
    }
    else
    {
        perror("PLAYER IS NOT EXIST");
        pthread_mutex_unlock(&Fd_Player_Lock);
        return -1;
    }
    pthread_mutex_unlock(&Fd_Player_Lock);
    return 0;
}