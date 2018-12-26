#include "ServerListener.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils/src/config.h"

using namespace simplefs;


const int BACKLOG_SIZE = 20;


ServerListener::ServerListener()
{
	unlink(DAEMON_SOCKET_PATH);
	socketFD = socket(AF_UNIX, SOCK_STREAM, 0);

	if (socketFD == -1)
		return;

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, DAEMON_SOCKET_PATH);

	if (bind(socketFD, (const sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1)
		return;

	if (listen(socketFD, BACKLOG_SIZE) == -1)
		return;

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
		return false;

	runExecutor(ret);

	return true;
}

bool ServerListener::isOk()
{
	return ok;
}

void simplefs::runExecutor(int connectionSocket)
{
	//TODO
	close(connectionSocket);
}
