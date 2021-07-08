#ifndef MYEPOLL
#define MYEPOLL

#include "requestData.h"

constexpr int MAXEVENTS = 5000;
constexpr int LISTENQ = 1024;

int epollInit();
int epollAdd(int efd, int fd, void* request, unsigned int events);
int epollWait(int efd, struct epoll_event* events, int maxEvents, int timeout);

#endif
