#include "IPCPackets.h"

#include <string.h>

using namespace simplefs;


int FDResponse::getBaseLength()
{
	return sizeof(unsigned int) + sizeof(int);
}

void FDResponse::deserializeBase(const char* data)
{
	data += sizeof(unsigned int);
	fd = *(const int*)data;
}

void FDResponse::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(int*)data = fd;
}

void FDResponse::setFD(int number)
{
	fd = number;
}

int FDResponse::getFD()
{
	return fd;
}
