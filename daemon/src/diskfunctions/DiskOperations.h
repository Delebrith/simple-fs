#pragma once

#include "daemon/src/management/DiskDescriptor.h"
#include "daemon/src/management/UsageMap.h"
#include "daemon/src/management/InodeList.h"
#include "daemon/src/management/Inode.h"

struct DiskOperations
{
    const unsigned int maxInodesCount; // = 100;
    const unsigned int blockSize; //  = 512;
    const unsigned int fsSize; //  = 512 * 100;

    DiskDescriptor* ds;
    UsageMap* um;
    InodeList* inodeList;
    unsigned char* shmaddr;
    int shmid;
    key_t key;

    int reallocate(Inode* inode, unsigned int newsize);

    unsigned char* getShmAddr(unsigned int blockIndex);

    int initShm();
    int initDiskStructures();
    int initRoot();

    DiskOperations(unsigned int inodesCount, unsigned int blockSize, unsigned int fsSize);
    virtual ~DiskOperations();

    int mkdir(char* path);
};