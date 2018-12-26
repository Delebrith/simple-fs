#include "ServerSessionConnector.h"

#include <unistd.h>

using namespace simplefs;


ServerSessionConnector::ServerSessionConnector(int socket)
{
	setSocket(socket);

	
	if (read(socket, &pid, sizeof(pid_t)) == -1)
		return;

	setOk(true);
}

pid_t ServerSessionConnector::getPid()
{
	return pid;
}
