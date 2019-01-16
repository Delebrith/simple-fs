#include "lib/src/simplefs.h"

#include <iostream>
#include <fcntl.h>

int main(int argc, char** argv)
{
    char* dir_path = "/dir";

    int ret = simplefs::simplefs_mkdir(dir_path, S_IWOTH | S_IROTH | S_IXOTH);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }


    char* path = "/dir/file";

    ret = simplefs::simplefs_create(path, 0);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    ret = simplefs::simplefs_close(ret);

    ret = simplefs::simplefs_chmode(dir_path, S_IROTH | S_IXOTH);

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
        if (err == EPERM)
            return 0;
        return -1;
    }

    return -1;
}