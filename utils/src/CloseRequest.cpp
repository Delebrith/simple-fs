#include "IPCPackets.h"

using namespace simplefs;


int CloseRequest::getBaseLength()
{
	return sizeof(unsigned int) + sizeof(int);
}

void CloseRequest::deserializeBase(const char* data)
{
	data += sizeof(unsigned int);
	fd = *(const int*)data;
}

void CloseRequest::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = fd;
}

int CloseRequest::getFD()
{
	return fd;
}

void CloseRequest::setFD(int fd)
{
	this->fd = fd;
}

