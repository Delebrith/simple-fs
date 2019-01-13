#pragma once
#include <time.h>
#include <sys/stat.h>



struct Inode
{
    const static unsigned int IT_FILE = 0;
    const static unsigned int IT_DIRECTORY = 1;

    const static unsigned int PERM_R = S_IROTH;
    const static unsigned int PERM_W = S_IWOTH;
    const static unsigned int PERM_X = S_IXOTH;

    unsigned int id;
    unsigned int nodeSize;
    unsigned int blockAddress; // block number
    unsigned int permissions;
    time_t creationDate;
    time_t modificationDate;
    time_t accessDate;
    time_t deletionDate;
    unsigned int fileType;
};