#ifndef UTIL
#define UTIL
#include <cstdlib>
#include <iostream>

using namespace std;

ssize_t readn(int fd, void *buff, size_t n);
ssize_t writen(int fd, void *buff, size_t n);
void handle_for_sigpipe();
int setSocketNonBlocking(int fd);

#endif