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

void* executor(void* connector)
{
	ServerSessionConnector* conn = static_cast<ServerSessionConnector*>(connector);

	Packet* request = conn->receive();
	Packet* response = nullptr;
	
	//log request
	//switch request
	//log response
	conn->send(*response);

	delete response;
	delete request;
	delete conn;
	return nullptr;
}
