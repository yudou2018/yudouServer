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

using namespace std;

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
	cout<<"Client: "<<inet_ntop(AF_INET,&client.sin_addr,remote,1024)<<":"<<to_string(ntohs(client.sin_port))<<endl;
	
	// the message to send should obey the HTTP type
	string str="<head><head><body><table border ='2'><tr><td>Visited: "+to_string(num)+"<td></tr><tr><td>Editor: Yudou</td></tr><tr><td>Message: Hello World!</td></tr><td>Date: July 7th, 2021<td></table></body>";
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
	if(argc!=3){
		printf("usage: %s ip_address port\n",basename(argv[0]));
		return -1;
	}

	// create socket and set nonblocking
	int listenfd = socket(AF_INET, SOCK_STREAM, 0); 
	setnonblocking(listenfd);	
	
	// put the ip and port to struct 
	const char* ip=argv[1]; // get the IP address
	int port = atoi(argv[2]); // get the port
	struct sockaddr_in address;
	bzero(&address,sizeof(address));
	address.sin_family = AF_INET; // use ipv4
	inet_pton(AF_INET,ip,&address.sin_addr); // input ip address to sin_addr
	address.sin_port=htons(port); // change port number to net short type
	
	// bind address information struct with socketfd
	bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	listen(listenfd,50); // add socketfd to listen queue
	
	// ev for register event, events for transmmit back event
	struct epoll_event ev,  events[20];
	int epfd=epoll_create(10000); // create epoll handler
	ev.data.fd=listenfd; // the event's fd is the listendfd
	ev.events=EPOLLIN; // read
	epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev); // add the event to epoll queue

	int connfd;
	// the struct to receive the client information
	struct sockaddr_in client;
	socklen_t client_len = sizeof(client);
	while(1)
	{
		int nfds=epoll_wait(epfd,events,20,1000); // maxevents=20 timeout=1000
		for(int i=0;i<nfds;++i)
		{
			if(events[i].data.fd==listenfd) // if it is connect request
			{
				// accept the connection
				cout<<"LISTEN_EVENT("<<events[i].data.fd<<")"<<endl;
				connfd=accept(listenfd,(struct sockaddr*)&client, &client_len);
				mp[connfd]=client;

				// log the connect ip port
				char ipbuff[32];
				cout<<"Connected: "<<inet_ntop(AF_INET, (void*)&client.sin_addr, ipbuff, sizeof(ipbuff))<<":"<<ntohs(client.sin_port)<<endl<<endl;
				
				setnonblocking(connfd);	
				ev.data.fd=connfd; // set the fd for read
				ev.events=EPOLLIN|EPOLLET; // the event for read
				epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev); // add the new connection fd to epoll queue
				
				//myProcess(connfd);
			}
			else if(events[i].events&EPOLLIN) // if it is read event
			{
				cout<<"EOLLIN_EVENT("<<events[i].data.fd<<")"<<endl<<endl;
				myProcess(events[i].data.fd);	
			}
			else if(events[i].events&EPOLLOUT) // if it is write event
			{
				cout<<"EPOLLOUT_EVENT("<<events[i].data.fd<<")"<<endl<<endl;
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
