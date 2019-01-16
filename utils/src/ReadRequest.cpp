#include "IPCPackets.h"

using namespace simplefs;

#include <iostream>

int ReadRequest::getBaseLength()
{
	return sizeof(unsigned int) + sizeof(int) + sizeof(int);
}

void ReadRequest::deserializeBase(const char* data)
{
	fd = *(const int*)data;
std::cout << "RECEIVED DESC: " << fd << std::endl;
	data += sizeof(int);	
	len = *(const int*)data;
std::cout << "RECEIVED LEN: " << len << std::endl;
}

void ReadRequest::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = fd;
std::cout << "SENT DESC: " << fd << std::endl;
	data += sizeof(int);
	*(int*)data = len;
std::cout << "SENT LEN: " << len << std::endl;
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

