#include "myEpoll.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <errno.h>

struct epoll_event* events;

int epollInit()
{
	int efd = epoll_create(LISTENQ+1);
	if(efd==-1)
	{
		return -1;
	}
	events = new epoll_event[MAXEVENTS];
	return efd;
}

int epollAdd(int epfd, int fd, void* request, unsigned int evFlag)
{
	struct epoll_event ev;
	ev.data.ptr = request;
	ev.events = evFlag;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD,fd,&ev)<0)
	{
		perror("epoll add error\n");
		return -1;
	}
	return 0;
}

int epollWait(int epfd, struct epoll_event* events, int maxEvents, int timeout)
{
	int nfd = epoll_wait(epfd, events, maxEvents, timeout);
	if(nfd<0)
	{
		perror("epoll wait error\n");
		return -1;
	}
	return nfd;
}
