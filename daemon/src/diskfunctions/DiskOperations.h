#pragma once

#include "daemon/src/management/DiskDescriptor.h"
#include "daemon/src/management/UsageMap.h"
#include "daemon/src/management/InodeList.h"
#include "daemon/src/management/Inode.h"
#include "utils/src/IPCPackets.h"

using namespace simplefs;

struct DiskOperations
{
    const unsigned int maxInodesCount; // = 100;
    const unsigned int blockSize; //  = 512;
    const unsigned int fsSize; //  = 512 * 100;
    const unsigned int volumeId;
    const char* volumeName;

    DiskDescriptor* ds;
    UsageMap* um;
    InodeList* inodeList;
    unsigned char* shmaddr;
    int shmid;
    key_t key;

    unsigned char* reallocate(Inode* inode, unsigned int newsize);

    unsigned char* getShmAddr(unsigned int blockIndex);

    Inode* getInodeById(unsigned int id);

    ErrorResponse* newErrorResponse(int errnum);

    int initShm();
    int initDiskStructures();
    int initRoot();

    void createNewDirEntry(int parentInodeId, int inodeBlockAddress, int inodeDataBlockAddress, int mode);

    DiskOperations(const char* volumeName, unsigned int volumeId, unsigned int maxInodesCount, unsigned int blockSize, unsigned int fsSize);
    virtual ~DiskOperations();

    Packet* mkdir(char* path, int mode);
    Packet* open(char* path, int mode);
    Packet* unlink(char* path);
    Packet* create(char* path, int mode);
    Packet* read(int fd, int len);
    Packet* write(int fd, int len);
    Packet* lseek(int fd, int offset, int whence);
    Packet* chmod(char* path, int mode);


    void printUsageMap();
    void printInodeParams(int i);
    void printInodes();
};
