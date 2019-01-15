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
	err = *(const int*)data;
}

void ErrorResponse::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = err;
}

void ErrorResponse::setErrno(int errno)
{
	this->err = err;
}

int ErrorResponse::getErrno()
{
	return err;
}

