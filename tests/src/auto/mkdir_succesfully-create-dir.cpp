#include "lib/src/simplefs.h"

#include <iostream>

int main(int argc, char** argv)
{
    char* path = "/dir";

    int ret = simplefs::simplefs_mkdir(path, 0);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    simplefs::simplefs_close(ret);

    std::cout << "Ok" << std::endl;
    return 0;
}