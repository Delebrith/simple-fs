#include "ClientConnector.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils/src/config.h"

using namespace simplefs;


ClientConnector::ClientConnector()
{
	socketFD = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socketFD == -1)
	{
		ok = false;
		return;
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, DAEMON_SOCKET_PATH);

	if (connect(socketFD, (const sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1)
	{
		ok = false;
		return;
	}
	
	ok = true;
}

ClientConnector::~ClientConnector()
{
	if (socketFD >= 0)
		close(socketFD);
}

int ClientConnector::send(Packet& packet)
{
	int size = packet.getTotalLength();
	char* buffer = new char[size];

	packet.serialize(buffer);

	if (write(socketFD, buffer, size) == -1)
	{
		delete[] buffer;
		return -1;
	}
	
	delete[] buffer;
	return 0;
}

Packet* ClientConnector::receive()
{
	unsigned int id;
	if (read(socketFD, &id, sizeof(unsigned int)) == -1)
		return nullptr;

	Packet* received = nullptr;
	if (id == OKResponse::ID)
		received = new OKResponse;
	else if (id == ErrorResponse::ID)
		received = new ErrorResponse;
	else if (id == ShmemPtrResponse::ID)
		received = new ShmemPtrResponse;

	if (received == nullptr)
		return nullptr;

	read(socketFD, basicBuffer, received->getBaseLength());
	if (received->getRemainderLength() == 0)
		return received;

	char* buf = received->getRemainderBuffer();
	if (read(socketFD, buf, received->getRemainderLength()) == -1)
	{
		delete received;
		return nullptr;
	}

	return received;
}

bool ClientConnector::isOk()
{
	return ok;
}

