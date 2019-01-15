#include "lib/src/simplefs.h"

#include <fcntl.h>

#include <iostream>
#include <cstdio>

#include <tests/src/utils/ls.h>

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: ls <path>" << std::endl;
		return -1;
	}

    try
    {
        std::vector<std::string> list = ls(argv[1]);

        for (const auto& entry : list)
            std::cout << entry << std::endl;
        
        return 0;
    }
    catch (std::exception e)
    {
        std::cout << "An error occured while trying to list directory.";
        return -1;
    }
}
