#pragma once

struct Inode
{
    unsigned int id;
    unsigned int nodeSize;
    unsigned int permissions;
    unsigned int creationDate;
    unsigned int modificationDate;
    unsigned int accessDate;
    unsigned int deletionDate;
    unsigned int fileType;
};