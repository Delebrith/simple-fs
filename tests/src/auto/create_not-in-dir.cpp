#include "lib/src/simplefs.h"

#include <iostream>

int main(int argc, char** argv)
{
    char* path = "/file1";

    int ret = simplefs::simplefs_create(path, 0);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    path = "/file1/file2";

    ret = simplefs::simplefs_create(path, 0);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        if (err == ENOTDIR)
            return 0;
        else
            return 1;
    }

    return -1;
}