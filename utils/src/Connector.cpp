#include "Connector.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "utils/src/config.h"

using namespace simplefs;


Connector::~Connector()
{
	if (socketFD >= 0)
		close(socketFD);
}

int Connector::send(Packet& packet)
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

Packet* Connector::receive()
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

void Connector::setSocket(int socket)
{
	ok = true;
	socketFD = socket;
}

bool Connector::isOk()
{
	return ok;
}

