#include "ServerSessionConnector.h"

#include <errno.h>
#include <unistd.h>

#include "daemon/src/logging/log.h"

using namespace simplefs;


ServerSessionConnector::ServerSessionConnector(int socket)
{
	setSocket(socket);

	if (read(socket, &pid, sizeof(pid_t)) == -1)
	{
		int tempErrno = errno;
		simplefs::log::logError("Server", "Failed to receive pid from socket with errno %d", tempErrno);
		errno = tempErrno;
		return;
	}

	log::logInfo("Connector", "New connection from process %d", pid);

	setOk(true);
}

pid_t ServerSessionConnector::getPid()
{
	return pid;
}
