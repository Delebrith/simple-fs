#include "lib/src/simplefs.h"

#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: unlink <path>" << std::endl;
		return -1;
	}

	int ret = simplefs::simplefs_unlink(argv[1]);

	if (ret != 0)
	{
		int err = errno;
		std::cout << "Error: " << err << std::endl;
		return -1;
	}

	std::cout << "Ok" << std::endl;
	return 0;
}
