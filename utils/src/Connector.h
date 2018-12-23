#pragma once

#include "IPCPackets.h"

namespace simplefs
{
	class Connector
	{
	public:
		~Connector();
		
		int send(Packet& packet);

		/*
		 * the new packet will be placed on stack,
		 * it will be responiblity of caller to release memory
		 */
		Packet* receive();

		bool isOk();

	protected:
		void setSocket(int socket);

		void setOk(bool);
	private:
		char basicBuffer[4 * sizeof(int)]; //enough for any packet except those with varying length
		int socketFD = -1;
		bool ok = false;
	};
}
