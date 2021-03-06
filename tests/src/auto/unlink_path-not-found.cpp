#include "lib/src/simplefs.h"

#include <iostream>

int main(int argc, char** argv)
{
    char* path = "/dir1/dir2";

    int ret = simplefs::simplefs_unlink(path);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;

        if (err == ENOENT)
            return 0;

        return -1;
    }


    std::cout << "Ok" << std::endl;
    return -1;
}
