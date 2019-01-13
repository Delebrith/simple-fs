#include "IPCPackets.h"

using namespace simplefs;


int ReadRequest::getBaseLength()
{
	return sizeof(unsigned int) + sizeof(int);
}

void ReadRequest::deserializeBase(const char* data)
{
	data += sizeof(unsigned int);
	fd = *(const int*)data;
	data += sizeof(int);	
	len = *(const int*)data;
}

void ReadRequest::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = fd;
	data += sizeof(int);
	*(int*)data = len;
}

int ReadRequest::getFD()
{
	return fd;
}

void ReadRequest::setFD(int fd)
{
	this->fd = fd;
}


int ReadRequest::getLen()
{
	return len;
}

void ReadRequest::setLen(int len)
{
	this->len = len;
}

