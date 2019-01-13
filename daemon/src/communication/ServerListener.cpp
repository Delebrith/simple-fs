#include "ServerListener.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <pthread.h>

#include "utils/src/config.h"
#include "daemon/src/logging/log.h"
#include "ServerSessionConnector.h"
#include "daemon/src/diskfunctions/DiskOperations.h"
#include "daemon/src/management/FileDescriptor.h"

using namespace simplefs;


const int BACKLOG_SIZE = 20;


ServerListener::ServerListener()
{
	unlink(DAEMON_SOCKET_PATH);
	socketFD = socket(AF_UNIX, SOCK_STREAM, 0);

	if (socketFD == -1)
	{
		int tempErrno = errno;
		simplefs::log::logError("Server", "Failed to create socket with errno %d", tempErrno);
		errno = tempErrno;
		return;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, DAEMON_SOCKET_PATH);

	if (bind(socketFD, (const sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1)
	{
		int tempErrno = errno;
		simplefs::log::logError("Server", "Failed to bind socket with errno %d", tempErrno);
		errno = tempErrno;
		return;
	}

	if (listen(socketFD, BACKLOG_SIZE) == -1)
	{
		int tempErrno = errno;
		simplefs::log::logError("Server", "Failed to start listening on socket with errno %d", tempErrno);
		errno = tempErrno;
		return;
	}

	ok = true;
}

ServerListener::~ServerListener()
{
	if (socketFD >= 0)
		close(socketFD);
}

bool ServerListener::waitForConnection()
{
	int ret = accept(socketFD, nullptr, nullptr);
	if (ret < 0)
	{
		int tempErrno = errno;
		log::logError("Server", "Failed to accept connection on socket with errno %d", tempErrno);
		errno = tempErrno;
		return false;
	}

	runExecutor(ret);

	return true;
}

bool ServerListener::isOk()
{
	return ok;
}

void* executor(void* connector);

void simplefs::runExecutor(int connectionSocket)
{
	ServerSessionConnector* conn = new ServerSessionConnector(connectionSocket);
	if (!conn->isOk())
	{
		simplefs::log::logError("Server", "Failed to receive connection");
		delete conn;
		return;
	}
	
	pthread_t thread;	
	pthread_create(&thread, nullptr, executor, conn);
	pthread_detach(thread);
}

extern DiskOperations* diskOps;
extern FileDescriptorTable* fdTable;

void* executor(void* connector)
{
	ServerSessionConnector* conn = static_cast<ServerSessionConnector*>(connector);

	Packet* request = conn->receive();
	Packet* response = nullptr;

	FileDescriptor* fd = nullptr;
	
	simplefs::log::logInfo("Executor", "Recived request %x from process %d", request->getId(), conn->getPid());

	switch(request->getId())
	{
	case OperationWithPathRequest::Create:
		response = diskOps->create(
			static_cast<OperationWithPathRequest*>(request)->getPath(),
			static_cast<OperationWithPathRequest*>(request)->getMode(),
			conn->getPid());
		break;
	case OperationWithPathRequest::Open:
		response = diskOps->open(
			static_cast<OperationWithPathRequest*>(request)->getPath(),
			static_cast<OperationWithPathRequest*>(request)->getMode(),
			conn->getPid());
		break;
	case OperationWithPathRequest::Chmd:
		response = diskOps->chmod(
			static_cast<OperationWithPathRequest*>(request)->getPath(),
			static_cast<OperationWithPathRequest*>(request)->getMode());
		break;
	case OperationWithPathRequest::Unlink:
		response = diskOps->unlink(
			static_cast<OperationWithPathRequest*>(request)->getPath());
		break;
	case OperationWithPathRequest::Mkdir:
		response = diskOps->mkdir(
			static_cast<OperationWithPathRequest*>(request)->getPath(),
			static_cast<OperationWithPathRequest*>(request)->getMode());
		break;
	case LSeekRequest::ID:
		fd = fdTable->getDescriptor(conn->getPid(), request->getId());
		if (fd == nullptr)
			response = new ErrorResponse(EBADF);
		else
		{
			response = diskOps->lseek(fd,
				static_cast<LSeekRequest*>(request)->getOffset(),
				static_cast<LSeekRequest*>(request)->getWhence());
		}
		break;
	case ReadRequest::ID:
		fd = fdTable->getDescriptor(conn->getPid(), request->getId());
		if (fd == nullptr)
			response = new ErrorResponse(EBADF);
		else
		{
			response = diskOps->read(fd,
				static_cast<ReadRequest*>(request)->getLen());
		}
		break;
	case WriteRequest::ID:
		fd = fdTable->getDescriptor(conn->getPid(), request->getId());
		if (fd == nullptr)
			response = new ErrorResponse(EBADF);
		else
		{
			response = diskOps->write(fd,
				static_cast<WriteRequest*>(request)->getLen());
		}
		break;
	case CloseRequest::ID:
		fd = fdTable->getDescriptor(conn->getPid(), request->getId());
		if (fd == nullptr)
			response = new ErrorResponse(EBADF);
		else
		{
			fdTable->destroyDescriptor(fd);
			response = new OKResponse();
		}
		break;
	}

	if (response == nullptr)
	{
		simplefs::log::logWarning("Executor", "Did not create response for request %x", request->getId());
	}
	else
	{
		simplefs::log::logInfo("Executor", "Sending response %x to process %d as reply to request %x",
				response->getId(),
				conn->getPid(),
				request->getId());
				
		conn->send(*response);
	}

	delete response;
	delete request;
	delete conn;
	return nullptr;
}
