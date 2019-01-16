#include "lib/src/simplefs.h"

#include <iostream>
#include <cstring>
#include <fcntl.h>

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

    const char* buf = "content";
    int ret = simplefs::simplefs_write(desc, buf, 8);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }
    simplefs::simplefs_close(desc);

    desc = simplefs::simplefs_open(path, O_RDONLY);

    if (desc < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    char read_buf[10];
    ret = simplefs::simplefs_read(desc, read_buf, 8);

    if (ret < 0)
    {
        int err = errno;
        std::cout << "Error: " << err << std::endl;
        return -1;
    }

    printf("buf: %s\n", buf);
    printf("rdbuf %d: %s\n", ret, read_buf);

    if (strcmp(buf, read_buf) == 0)
        return 0;

    simplefs::simplefs_close(desc);

    std::cout << "Ok" << std::endl;
    return -1;
}