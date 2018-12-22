#include "IPCPackets.h"

using namespace simplefs;


int OKResponse::getBaseLength()
{
	return sizeof(unsigned int);
}

void OKResponse::deserializeBase(const char* data)
{
}

void OKResponse::serialize(char* data)
{
	*(unsigned int*)data = ID;
}

