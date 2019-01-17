#include "IPCPackets.h"

using namespace simplefs;


int LSeekRequest::getBaseLength()
{
	return sizeof(unsigned int) + 3 * sizeof(int);
}

void LSeekRequest::deserializeBase(const char* data)
{
	fd = *(const int*)data;
	data += sizeof(int);
	offset = *(const int*)data;
	data += sizeof(int);
	whence = *(const int*)data;
}

void LSeekRequest::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = fd;
	data += sizeof(int);
	*(int*)data = offset;
	data += sizeof(int);
	*(int*)data = whence;
}

int LSeekRequest::getFD()
{
	return fd;
}

void LSeekRequest::setFD(int fd)
{
	this->fd = fd;
}

int LSeekRequest::getOffset()
{
	return offset;
}

void LSeekRequest::setOffset(int offset)
{
	this->offset = offset;
}

int LSeekRequest::getWhence()
{
	return whence;
}

void LSeekRequest::setWhence(int whence)
{
	this->whence = whence;
}

