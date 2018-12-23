#include "ClientConnector.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils/src/config.h"

using namespace simplefs;


ClientConnector::ClientConnector()
{
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock == -1)
		return;

	setSocket(sock);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, DAEMON_SOCKET_PATH);

	if (connect(sock, (const sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1)
		return;
	
	pid_t pid = getpid();
	if (write(sock, &pid, sizeof(pid_t)) == -1)
		return;

	setOk(false);
}

