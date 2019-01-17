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

    ret = simplefs::simplefs_unlink(path);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        if (err == EBUSY)
            return 0;
    }

    std::cout << "Ok" << std::endl;
    return -1;
}