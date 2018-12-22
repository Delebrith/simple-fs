#pragma once

#include <sys/types.h>

#include "utils/src/Connector.h"


namespace simplefs
{
	class ServerSessionConnector : public Connector
	{
	public:
		ServerSessionConnector(int socket);

		pid_t getPid();
	private:
		pid_t pid;
	};
}
