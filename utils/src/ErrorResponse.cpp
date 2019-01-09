#include "IPCPackets.h"

#include <string.h>

using namespace simplefs;


int ErrorResponse::getBaseLength()
{
	return sizeof(unsigned int) + sizeof(int);
}

void ErrorResponse::deserializeBase(const char* data)
{
	data += sizeof(unsigned int);
	errno = *(const int*)data;
}

void ErrorResponse::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = errno;
}

void ErrorResponse::setErrno(int errno)
{
	this->errno = errno;
}

int ErrorResponse::getErrno()
{
	return errno;
}

