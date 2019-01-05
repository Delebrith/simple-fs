#pragma once
#include <time.h>



struct Inode
{
    const static unsigned int IT_FILE = 0;
    const static unsigned int IT_DIRECTORY = 1;

    const static unsigned int PERM_R = 4;
    const static unsigned int PERM_W = 2;
    const static unsigned int PERM_X = 1;

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