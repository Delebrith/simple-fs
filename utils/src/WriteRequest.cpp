#include "IPCPackets.h"

using namespace simplefs;

int WriteRequest::getBaseLength()
{
	return sizeof(unsigned int) + 2 * sizeof(int);
}

void WriteRequest::deserializeBase(const char* data)
{
	fd = *(const int*)data;
	data += sizeof(int);
	len = *(const unsigned int*)data;
}

void WriteRequest::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = fd;
	data += sizeof(int);
	*(unsigned int*)data = len;
}

int WriteRequest::getFD()
{
	return fd;
}

void WriteRequest::setFD(int fd)
{
	this->fd = fd;
}

int WriteRequest::getLen()
{
	return len;
}

void WriteRequest::setLen(int len)
{
	this->len = len;
}

