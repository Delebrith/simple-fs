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
}

void ReadRequest::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = fd;
}

int ReadRequest::getFD()
{
	return fd;
}

void ReadRequest::setFD(int fd)
{
	this->fd = fd;
}

