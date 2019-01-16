#include "simplefs.h"

#include <errno.h>
#include <string.h>
#include <sys/shm.h>

#include "ClientConnector.h"
#include <utils/src/IPCPackets.h>

using namespace simplefs;

#include <iostream>


Packet* send(Packet& req)
{
	ClientConnector conn;
	if (!conn.isOk())
		return nullptr;

	if (conn.send(req) == -1)
		return nullptr;

	return conn.receive();
}

int checkOkErrorResponse(Packet* response)
{
	int retval = -1;

	if (response->getId() == ErrorResponse::ID)
	{
		errno = dynamic_cast<ErrorResponse*>(response)->getErrno();
	}
	else if (response->getId() == OKResponse::ID)
	{
		retval = 0;
	}
	
	delete response;
	return retval;
}

namespace simplefs
{
	EXTERN_C int simplefs_open(const char* path, int mode)
	{
		OperationWithPathRequest req(OperationWithPathRequest::Open);
		req.setMode(mode);
		req.setPath(path);

		Packet* response = send(req);
		if (response == nullptr)
			return -1;
	
		int fd = -1;

		if (response->getId() == ErrorResponse::ID)
		{
			errno = dynamic_cast<ErrorResponse*>(response)->getErrno();
		}
		else if (response->getId() == FDResponse::ID)
		{
			fd = dynamic_cast<FDResponse*>(response)->getFD();
		}
	
		delete response;
		return fd;
	}

	EXTERN_C int simplefs_unlink(const char* path)
	{
		OperationWithPathRequest req(OperationWithPathRequest::Unlink);
		req.setPath(path);
	
		Packet* response = send(req);
		if (response == nullptr)
			return -1;

		return checkOkErrorResponse(response);
	}

	EXTERN_C int simplefs_mkdir(const char* path, int mode)
	{
		OperationWithPathRequest req(OperationWithPathRequest::Mkdir);
		req.setPath(path);
		req.setMode(mode);
	
		Packet* response = send(req);
		if (response == nullptr)
			return -1;

		return checkOkErrorResponse(response);
	}

	EXTERN_C int simplefs_create(const char* path, int mode)
	{
		OperationWithPathRequest req(OperationWithPathRequest::Create);
		req.setPath(path);
		req.setMode(mode);
	
		Packet* response = send(req);
		if (response == nullptr)
			return -1;

		int fd = -1;

		if (response->getId() == ErrorResponse::ID)
		{
			errno = dynamic_cast<ErrorResponse*>(response)->getErrno();
		}
		else if (response->getId() == FDResponse::ID)
		{
			fd = dynamic_cast<FDResponse*>(response)->getFD();
		}
	
		delete response;
		return fd;
	}

	EXTERN_C int simplefs_read(int fd, char* buf, int len)
	{
		ReadRequest req;
		req.setFD(fd);
		req.setLen(len);
	
		Packet* response = send(req);
		if (response == nullptr)
			return -1;

		if (response->getId() == ShmemPtrResponse::ID)
		{
			ShmemPtr ptr = dynamic_cast<ShmemPtrResponse*>(response)->getPtr();
			
			void* shmemptr = shmat(ptr.shmid, 0, 0);
std::cout << "SHMEMPTR " << shmemptr << std::endl;
			if (shmemptr == (void*)-1)
				return -1;

std::cout << "[" << len << ":" << ptr.size << "]" << std::endl;

			int toRead = len > ptr.size ? ptr.size : len;
			memcpy(buf, (char*)shmemptr + ptr.offset, toRead);

			shmdt(shmemptr);
			delete response;
std::cout << "READ: " << toRead;
			return toRead;
		}

		if (response->getId() == ErrorResponse::ID)
			errno = dynamic_cast<ErrorResponse*>(response)->getErrno();

		delete response;
		return -1;
	}

	EXTERN_C int simplefs_write(int fd, const char* buf, int len)
	{
		WriteRequest req;
		req.setFD(fd);
		req.setLen(len);
	
		Packet* response = send(req);
		if (response == nullptr)
			return -1;

		if (response->getId() == ShmemPtrResponse::ID)
		{
			ShmemPtr ptr = dynamic_cast<ShmemPtrResponse*>(response)->getPtr();
			
			void* shmemptr = shmat(ptr.shmid, 0, 0);
			if (shmemptr == (void*)-1)
				return -1;

			int toWrite = len > ptr.size ? ptr.size : len;
			memcpy((char*)shmemptr + ptr.offset, buf, toWrite);

			shmdt(shmemptr);
			delete response;
			return toWrite;
		}

		if (response->getId() == ErrorResponse::ID)
			errno = dynamic_cast<ErrorResponse*>(response)->getErrno();

		delete response;
		return -1;
	}

	EXTERN_C int simplefs_lseek(int fd, int offset, int whence)
	{
		LSeekRequest req;
		req.setFD(fd);
		req.setOffset(offset);
		req.setWhence(whence);
	
		Packet* response = send(req);
		if (response == nullptr)
			return -1;

		return checkOkErrorResponse(response);
	}

	EXTERN_C int simplefs_chmode(const char* path, mode_t mode)
	{
		OperationWithPathRequest req(OperationWithPathRequest::Chmd);
		req.setPath(path);
		req.setMode(mode);
	
		Packet* response = send(req);
		if (response == nullptr)
			return -1;

		return checkOkErrorResponse(response);
	}

	EXTERN_C int simplefs_close(int fd)
	{
		CloseRequest req;
		req.setFD(fd);
	
		Packet* response = send(req);
		if (response == nullptr)
			return -1;

		return checkOkErrorResponse(response);
	}

}

