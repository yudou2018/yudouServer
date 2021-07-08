#include "requestData.h"

requestData::requestData():fd(-1)
{

}

requestData::requestData(int _fd):fd(_fd)
{
	
}

void requestData::setfd(int _fd)
{
	fd = _fd;
}

int requestData::getfd()
{
	return fd;
}
