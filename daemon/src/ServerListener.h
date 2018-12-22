#pragma once

namespace simplefs
{
	class ServerListener
	{
	public:
		ServerListener();
		~ServerListener();

		bool waitForConnection();
		bool isOk();
	private:
		int socketFD = -1;
		bool ok = false;

	};
	
	void runExecutor(int connectionSocket);
}
