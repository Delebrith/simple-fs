#include "IPCPackets.h"

using namespace simplefs;


int Packet::getTotalLength()
{
	return this->getBaseLength() + this->getRemainderLength();
}

int Packet::getId()
{
	return id;
}

Packet* Packet::fromId(int id)
{
	if (id == OperationWithPathRequest::Create)	return new OperationWithPathRequest(OperationWithPathRequest::Create);
	if (id == OperationWithPathRequest::Open)	return new OperationWithPathRequest(OperationWithPathRequest::Open);
	if (id == OperationWithPathRequest::Chmd)	return new OperationWithPathRequest(OperationWithPathRequest::Chmd);
	if (id == OperationWithPathRequest::Unlink)	return new OperationWithPathRequest(OperationWithPathRequest::Unlink);
	if (id == OperationWithPathRequest::Mkdir)	return new OperationWithPathRequest(OperationWithPathRequest::Mkdir);
	if (id == LSeekRequest::ID)			return new LSeekRequest;
	if (id == ReadRequest::ID)			return new ReadRequest;
	if (id == WriteRequest::ID)			return new WriteRequest;
	if (id == CloseRequest::ID)			return new CloseRequest;
	if (id == OKResponse::ID)			return new OKResponse;
	if (id == ErrorResponse::ID)			return new ErrorResponse;
	if (id == ShmemPtrResponse::ID)			return new ShmemPtrResponse;
	if (id == FDResponse::ID)			return new FDResponse;

	return nullptr;
}

