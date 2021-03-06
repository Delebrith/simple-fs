#include "IPCPackets.h"

using namespace simplefs;


int ShmemPtrResponse::getBaseLength()
{
	return sizeof(unsigned int) + sizeof(ShmemPtr);
}

void ShmemPtrResponse::deserializeBase(const char* data)
{
	ptr = *(const ShmemPtr*)data;
}

void ShmemPtrResponse::serialize(char* data)
{
	*(unsigned int*)data = ID;
	data += sizeof(unsigned int);
	*(ShmemPtr*)data = ptr;
}

ShmemPtr ShmemPtrResponse::getPtr()
{
	return ptr;
}
void ShmemPtrResponse::setPtr(ShmemPtr ptr)
{
	this->ptr = ptr;
}