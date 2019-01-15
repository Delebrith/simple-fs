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

#include <iostream>

int Connector::send(Packet& packet)
{
	int size = packet.getTotalLength();
	char* buffer = new char[size];

	packet.serialize(buffer);

std::cout << "BUF:" << buffer << std::endl;

	if (write(socketFD, buffer, size) == -1)
	{
		delete[] buffer;
		return -1;
	}
	
	delete[] buffer;
	return 0;
}
#include <iostream>
Packet* Connector::receive()
{
	unsigned int id;
std::cout << "R\n";
	if (read(socketFD, &id, sizeof(unsigned int)) == -1)
		return nullptr;
std::cout << "Read " << id << std::endl;
	Packet* received = Packet::fromId(id);

	if (received == nullptr)
		return nullptr;
std::cout << "Read2\n";
	read(socketFD, basicBuffer, received->getBaseLength());
	if (received->getRemainderLength() == 0)
		return received;
std::cout << "Read3\n";
	char* buf = received->getRemainderBuffer();
	if (read(socketFD, buf, received->getRemainderLength()) == -1)
	{
		delete received;
		return nullptr;
	}
std::cout << "Read4\n";
	return received;
}

void Connector::setSocket(int socket)
{
	socketFD = socket;
}

void Connector::setOk(bool ok)
{
	this->ok = ok;
}

bool Connector::isOk()
{
	return ok;
}

