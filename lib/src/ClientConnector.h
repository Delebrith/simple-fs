#pragma once

#include "utils/src/IPCPackets.h"


namespace simplefs
{
	class ClientConnector
	{
	public:
		ClientConnector();
		~ClientConnector();
		
		int send(Packet& packet);

		/*
		 * the new packet will be placed on stack,
		 * it will be responiblity of caller to release memory
		 */
		Packet* receive();

		bool isOk();
	
	private:
		char basicBuffer[4 * sizeof(int)]; //enough for any packet except those with varying length
		int socketFD;
		bool ok;
	};
}

