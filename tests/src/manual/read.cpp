#include "lib/src/simplefs.h"

#include <fcntl.h>

#include <iostream>
#include <cstdio>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: read <path>" << std::endl;
		return -1;
	}

	int fd = simplefs::simplefs_open(argv[1], O_RDONLY);

	if (fd < 0)
	{
		int err = errno;
		std::cout << "Error while opening: " << err << std::endl;
		return -1;
	}
std::cout << "FD: " << fd << std::endl;
	char buf[32];
	int ret;
	while (ret = simplefs::simplefs_read(fd, buf, 32))
	{
		if (ret < 0)
		{
			int err = errno;
			std::cout << "Error: " << err << std::endl;
			simplefs::simplefs_close(fd);
			return -1;
		}
		printf("\n\n");
		for (int i = 0; i < ret; ++i)
			printf("%02X", buf[i]);

		printf("\n");
		printf("\n\n");
	}

	simplefs::simplefs_close(fd);

	std::cout << "Ok" << std::endl;
	return 0;
}
