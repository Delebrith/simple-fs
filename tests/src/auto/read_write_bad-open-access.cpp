#include "lib/src/simplefs.h"

#include <iostream>
#include <cstring>
#include <fcntl.h>

int write_with(char* path, mode_t mode, int expected)
{
    int desc = simplefs::simplefs_open(path, mode);
    if (desc < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    char* buf = "buf";
    int ret = simplefs::simplefs_write(desc, buf, 3);

    if (expected != 0)
    {
        if (ret < 0)
        {
            int err = errno;
            std::cout << "Error: " << err << std::endl;
            if (err == expected)
            {
                simplefs::simplefs_close(desc);
                return 0;
            }
            simplefs::simplefs_close(desc);
            return -1;
        }
        else {
            simplefs::simplefs_close(desc);
            return -1;
        }
    }
    else {
        if (ret < 0)
        {
            int err = errno;
            std::cout << "Error: " << err << std::endl;
            simplefs::simplefs_close(desc);
            return -1;
        }
        simplefs::simplefs_close(desc);
        return 0;
    }
}


int read_with(char* path, mode_t mode, int expected)
{
    int desc = simplefs::simplefs_open(path, mode);
    if (desc < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    char buf[10];
    int ret = simplefs::simplefs_read(desc, buf, 3);

    if (expected != 0)
    {
        if (ret < 0)
        {
            int err = errno;
            std::cout << "Error: " << err << std::endl;
            if (err == expected)
            {
                simplefs::simplefs_close(desc);
                return 0;
            }
            simplefs::simplefs_close(desc);
            return -1;
        }
        else {
            simplefs::simplefs_close(desc);
            return -1;
        }
    }
    else {
        if (ret < 0)
        {
            int err = errno;
            std::cout << "Error: " << err << std::endl;
            simplefs::simplefs_close(desc);
            return -1;
        }
        simplefs::simplefs_close(desc);
        return 0;
    }
}

int main(int argc, char** argv)
{
    char* path = "/file";

    int desc = simplefs::simplefs_create(path, 0);

    if (desc < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    simplefs::simplefs_close(desc);

    if (write_with(path, O_RDONLY, EBADF) != 0)
        return -1;

    if (write_with(path, O_WRONLY, 0) != 0)
        return -1;

    if (write_with(path, O_RDWR, 0) != 0)
        return -1;

    if (read_with(path, O_RDONLY, 0) != 0)
        return -1;

    if (read_with(path, O_RDWR, 0) != 0)
        return -1;

    if (read_with(path, O_WRONLY, 0) != 0)
        return -1;

    return 0;
}