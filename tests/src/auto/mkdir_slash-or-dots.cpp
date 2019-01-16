#include "lib/src/simplefs.h"

#include <iostream>

int main(int argc, char **argv) {
    char *path = "/";

    int ret = simplefs::simplefs_mkdir(path, 0);

    if (ret < 0) {
        int err = errno;
        std::cout << "Error: " << err << std::endl;

        if (err != EEXIST)
            return -1;
    } else {
        return -1;
    }

    path = "/.";

    ret = simplefs::simplefs_mkdir(path, 0);

    if (ret < 0) {
        int err = errno;
        std::cout << "Error: " << err << std::endl;

        if (err != EEXIST)
            return -1;
    } else {
        return -1;
    }

    path = "/..";

    ret = simplefs::simplefs_mkdir(path, 0);

    if (ret < 0) {
        int err = errno;
        std::cout << "Error: " << err << std::endl;

        if (err == EEXIST)
            return 0;
    } else {
        return -1;
    }

}