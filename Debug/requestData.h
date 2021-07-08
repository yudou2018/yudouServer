#ifndef REQUESTDATA
#define REQUESTDATA

class requestData
{
	int fd;
public:
	requestData();
	requestData(int _fd);
	void setfd(int _fd);
	int getfd();
};

#endif
