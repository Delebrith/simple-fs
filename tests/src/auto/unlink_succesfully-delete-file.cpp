#include "lib/src/simplefs.h"

#include <iostream>

int main(int argc, char** argv)
{
    char* path = "/file";

    int ret = simplefs::simplefs_create(path, 0);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    simplefs::simplefs_close(ret);

    ret = simplefs::simplefs_unlink(path);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    std::cout << "Ok" << std::endl;
    return 0;
}