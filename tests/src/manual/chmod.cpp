#include "lib/src/simplefs.h"

#include <iostream>

int main(int argc, char** argv)
{
	int mode = 0;
	if (argc != 3)
	{
		std::cout << "Usage: chmod <path> <[rwx]+>" << std::endl;
		return -1;
	}

	for (int i = 0; argv[2][i]; ++i)
	{
		switch(argv[2][i])
		{
		case 'r':
			mode |= S_IROTH;
			break;
		case 'w':
			mode |= S_IWOTH;
			break;
		case 'x':
			mode |= S_IXOTH;
			break;
		default:
			std::cout << "Usage: chmod <path> <[rwx]+>" << std::endl;
			return -1;
		}
	}

	int ret = simplefs::simplefs_chmode(argv[1], mode);

	if (ret != 0)
	{
		int err = errno;
		std::cout << "Error: " << err << std::endl;
		return -1;
	}

	std::cout << "Ok" << std::endl;
	return 0;
}
