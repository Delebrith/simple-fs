#include "IPCPackets.h"

#include <string.h>

using namespace simplefs;

OperationWithPathRequest::~OperationWithPathRequest()
{
	delete[] this->path;
}

int OperationWithPathRequest::getBaseLength()
{
	return 2 * sizeof(unsigned int) + sizeof(int);
}

int OperationWithPathRequest::getRemainderLength()
{
	return pathLen;
}

void OperationWithPathRequest::deserializeBase(const char* data)
{
	data += sizeof(unsigned int);

	mode = *(const int*)data;
	data += sizeof(int);

	pathLen = *(const unsigned int*)data;
}

char* OperationWithPathRequest::getRemainderBuffer()
{
	delete[] path;
	path = new char[pathLen + 1];
	path[pathLen] = 0;
	return path;
}
#include <iostream>
void OperationWithPathRequest::serialize(char* data)
{
	*(unsigned int*)data = type;
std::cout << type;
	data += sizeof(unsigned int);
	*(int*)data = mode;
	data += sizeof(int);
	*(unsigned int*)data = pathLen;
	data += sizeof(unsigned int);
	memcpy(data, path, pathLen);
}

int OperationWithPathRequest::getMode()
{
	return mode;
}

void OperationWithPathRequest::setMode(int mode)
{
	this->mode = mode;
}


const char* OperationWithPathRequest::getPath()
{
	return path;
}

void OperationWithPathRequest::setPath(const char* path)
{
	delete[] this->path;
	pathLen = strlen(path);
	this->path = new char[pathLen + 1];
	strcpy(this->path, path);
}

OperationWithPathRequest::OpType OperationWithPathRequest::getType()
{
	return type;
}


