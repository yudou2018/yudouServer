#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h> 
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include <unordered_map>
#include <signal.h>

#include "myEpoll.h"

using namespace std;

constexpr int PORT = 12345;

extern struct epoll_event* events;

int bindListen()
{
	int listenfd = socket(AF_INET,SOCK_STREAM,0); // create listenfd
	if(listenfd == -1)
	{
		perror("socket error\n");
		return -1;
	}
	struct sockaddr_in address;
	bzero(&address,sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY); // any ip of localhost
	address.sin_port = htons(PORT);
	int ret = bind(listenfd,(struct sockaddr*)&address,sizeof(address)); // bind ip and port
	if(ret == -1)
	{
		perror("bind error\n");
		return -1;
	}
	ret = listen(listenfd, LISTENQ); // add to listen queue
	if(ret == -1)
	{
		perror("listen error\n");
		return -1;
	}
	return listenfd;
}

unordered_map<int,struct sockaddr_in> mp;

void setnonblocking(int& fd)
{
	// set fd to nonblocking
	int flags=fcntl(fd,F_GETFL,0);
	fcntl(fd,F_SETFL,flags|O_NONBLOCK);
}

int num=1;
void myProcess(int connfd)
{
	struct sockaddr_in client = mp[connfd];

	// inet_ntop transfer the ip to string, return the string
	// ntohs transfer the net type port to local short type port, return it
	char remote[1024];
	cout<<"Processed Client: "<<inet_ntop(AF_INET,&client.sin_addr,remote,1024)<<":"<<to_string(ntohs(client.sin_port))<<endl;
	
	// the message to send should obey the HTTP type
	string str="<head><head><body><table border ='2'><tr><td>Visited: "+to_string(num)+"<td></tr><tr><td>Editor: Yudou</td></tr><tr><td>Message: Hello World!</td></tr><td>Date: July 8th, 2021<td></table></body>";
	string httpStr="HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: "+to_string(str.size())+"\r\n\r\n"+str;

	// save the message in buff
	char buff[httpStr.size()];
	for(int j=0;j<httpStr.size();++j)
	{
		buff[j]=httpStr[j];
	}

	// send the content int buf to connfd aka clinet
	size_t msg_len = send(connfd,buff,sizeof(buff),0);
	bzero(buff,sizeof(buff));
	printf("send %d bytes message\n",msg_len);
	cout<<endl;

	++num; // the num will be added every time client connect
}


int main(int argc, char* argv[])
{
	signal(SIGPIPE,SIG_IGN);
	daemon(0,0);

	int listenfd = bindListen();
	int epfd = epollInit();
	if(epfd<0)
	{
		perror("epoll init error\n");
		return -1;
	}
	unsigned int eventFlag = EPOLLIN | EPOLLET;
	requestData* req = new requestData(listenfd);
	epollAdd(epfd, listenfd, (void*)req, eventFlag);
	
	int connfd;
	// the struct to receive the client information
	struct sockaddr_in client;
	socklen_t client_len = sizeof(client);
	while(1)
	{
		int nfds = epollWait(epfd,events,MAXEVENTS,-1); // maxevents=50000 timeout=-1
		cout<<"nfds: "<<nfds << endl;
		for(int i=0;i<nfds;++i)
		{
			req = (requestData*)events[i].data.ptr;
			if(req->getfd()==listenfd) // if it is connect request
			{
				// accept the connection
				cout<<"LISTEN_EVENT("<<req->getfd()<<")"<<endl;
				connfd = accept(listenfd,(struct sockaddr*)&client, &client_len);
				setnonblocking(connfd);	
				mp[connfd] = client;
				cout<<"set into map: "<<connfd<<endl;

				// log the connect ip port
				char ipbuff[32];
				cout<<"Connected: "<<inet_ntop(AF_INET, (void*)&client.sin_addr, ipbuff, sizeof(ipbuff))<<":"<<ntohs(client.sin_port)<<endl;
				// eventFlag = EPOLLIN | EPOLLET; // the event for read
				requestData* tmpPtr = new requestData(connfd);
				epollAdd(epfd,connfd,(void*)tmpPtr,eventFlag); // add the new connection fd to epoll queue	

				// myProcess(connfd);
			}
			else if(events[i].events&EPOLLIN) // if it is read event
			{
				req = (requestData*)events[i].data.ptr;
				cout<<"EOLLIN_EVENT("<<req->getfd()<<")"<<endl;
				myProcess(req->getfd());	
			}
			else if(events[i].events&EPOLLOUT) // if it is write event
			{
				cout<<"EPOLLOUT_EVENT("<<events[i].data.fd<<")"<<endl;
				cout<<"to wirte"<<endl;	
			}
			else
			{
				cout<<"others"<<endl;
			}
		}
	}

	return 0;
}
