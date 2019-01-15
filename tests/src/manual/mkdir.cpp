#include "lib/src/simplefs.h"

#include <iostream>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: mkdir <path>" << std::endl;
		return -1;
	}

	int ret = simplefs::simplefs_mkdir(argv[1], S_IROTH | S_IXOTH | S_IWOTH);

	if (ret != 0)
	{
		int err = errno;
		std::cout << "Error: " << err << std::endl;
		return -1;
	}

	std::cout << "Ok" << std::endl;
	return 0;
}
