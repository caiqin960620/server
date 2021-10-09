#ifndef MESSAGE
#define MESSAGE

#include <iostream>
#include <string>

using namespace std;

class CSMsg
{
public:
    CSMsg();
    CSMsg(int _fd, int _epoll_fd);
    ~CSMsg();
    int handleRequest();
    void setFd(int _fd);
    int getFd();
private:
	int fd;
    int epoll_fd;
    std::string message;
};

#endif