#include "lib/src/simplefs.h"

int main(int argc, char** argv)
{
	if (argv != 2)
	{
		std::cout << "Usage: write <path> <text>" << std::endl;
		return -1;
	}

	fd = simplefs::simplefs_open(argv[1], O_WRONLY);

	if (fd < 0)
	{
		int err = errno;
		std::cout << "Error while opening: " << err << std::endl;
		return -1;
	}

	int len = strlen(argv[2]);
	int ret = simplefs::simplefs_write(fd, argv[2], len)
	
	if (ret < 0)
	{
		int err = errno;
		std::cout << "Error: " << err << std::endl;
		simplefs::simplefs_close(fd);
		return -1;
	}
	
	simplefs::simplefs_close(fd);

	std::cout << "Ok" << std::endl;
	return 0;
}
