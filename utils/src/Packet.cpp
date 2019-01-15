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
	switch(id)
	{
	case OperationWithPathRequest::Create:	return new OperationWithPathRequest(OperationWithPathRequest::Create);
	case OperationWithPathRequest::Open:	return new OperationWithPathRequest(OperationWithPathRequest::Open);
	case OperationWithPathRequest::Chmd:	return new OperationWithPathRequest(OperationWithPathRequest::Chmd);
	case OperationWithPathRequest::Unlink:	return new OperationWithPathRequest(OperationWithPathRequest::Unlink);
	case OperationWithPathRequest::Mkdir:	return new OperationWithPathRequest(OperationWithPathRequest::Mkdir);
	case LSeekRequest::ID:			return new LSeekRequest;
	case ReadRequest::ID:			return new ReadRequest;
	case WriteRequest::ID:			return new WriteRequest;
	case CloseRequest::ID:			return new CloseRequest;
	case OKResponse::ID:			return new OKResponse;
	case ErrorResponse::ID:			return new ErrorResponse;
	case ShmemPtrResponse::ID:		return new ShmemPtrResponse;
	case FDResponse::ID:			return new FDResponse;
	}

	return nullptr;
}

