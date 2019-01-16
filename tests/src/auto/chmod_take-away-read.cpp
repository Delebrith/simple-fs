#include "lib/src/simplefs.h"

#include <iostream>
#include <fcntl.h>

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

    ret = simplefs::simplefs_chmode(path, S_IWOTH | S_IXOTH);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    ret = simplefs::simplefs_open(path, O_RDONLY);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        if (err == EACCES)
            return 0;
    }

    std::cout << "Ok" << std::endl;
    return -1;
}
