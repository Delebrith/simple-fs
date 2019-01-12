#pragma once

#include "daemon/src/management/DiskDescriptor.h"
#include "daemon/src/management/UsageMap.h"
#include "daemon/src/management/InodeList.h"
#include "daemon/src/management/Inode.h"
#include "daemon/src/management/FileDescriptor.h"
#include "utils/src/IPCPackets.h"
#include <semaphore.h>

namespace simplefs
{

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

        void createNewInodeEntry(unsigned int parentInodeId, int inodeBlockAddress, int inodeDataBlockAddress, int mode, int inodeFileType);

        DiskOperations(const char* volumeName, unsigned int volumeId, unsigned int maxInodesCount, unsigned int blockSize, unsigned int fsSize);
        virtual ~DiskOperations();

        int fillInodeWithDirectoryData();
        Packet* dirNavigate(const char* path, Inode** resultingInode);

        Packet* createInode(const char* path, int mode, int inodeFileType);
        Packet* mkdir(const char* path, int mode);
        Packet* open(const char* path, int mode);
        Packet* unlink(const char* path);
        Packet* create(const char* path, int mode);
        Packet* read(FileDescriptor* fd);
        Packet* write(FileDescriptor* fd, int len);
        Packet* lseek(FileDescriptor* fd, int offset, int whence);
        Packet* chmod(const char* path, int mode);


        void printUsageMap();
        void printInodeParams(int i);
        void printInodes();
    private:
        sem_t inodeOpSemaphore;
    };
}
