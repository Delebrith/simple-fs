#include "lib/src/simplefs.h"

#include <sys/stat.h>

#include <iostream>
#include <cstdio>

int main(int argc, char** argv)
{
	if (argv != 2)
	{
		std::cout << "Usage: read <path>" << std::endl;
		return -1;
	}

	fd = simplefs::simplefs_open(argv[1], O_RDONLY);

	if (fd < 0)
	{
		int err = errno;
		std::cout << "Error while opening: " << err << std::endl;
		return -1;
	}

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
		
		for (int i = 0; i < ret; ++i)
			printf("%02X", buf[i]);

		printf("\n");
	}

	simplefs::simplefs_close(fd);

	std::cout << "Ok" << std::endl;
	return 0;
}
