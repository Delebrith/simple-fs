//
// Created by llepak on 1/4/19.
//

#include <sys/ipc.h>
#include "DiskOperations.h"
#include "../management/Directory.h"
#include <errno.h>
#include <cstdio>
#include <sys/shm.h>
#include <cstring>
#include <time.h>

unsigned int ceil(unsigned int up, unsigned int down)
{
    unsigned int result = up / down;
    if (up % down != 0)
        ++result;
    return result;
}

int DiskOperations::reallocate(Inode* inode, unsigned int newSize)
{
    unsigned int oldBlocks = ceil(inode->nodeSize, blockSize);
    unsigned int newBlocks = ceil(newSize, blockSize);
    if (oldBlocks == newBlocks)
    {
        inode->modificationDate = time(0);
        inode->nodeSize = newSize;
        return 0;
    }
    else if (oldBlocks > newBlocks)
    {
        inode->modificationDate = time(0);
        inode->nodeSize = newSize;
        um->markBlocks(inode->blockAddress + newBlocks, inode->blockAddress + oldBlocks, true);
        return 0;
    }
    else
    {
        bool canExtend = true;
        for (unsigned int x = inode->blockAddress + oldBlocks; x < inode->blockAddress + newBlocks; ++x)
        {
            if (um->blocks[x] == UsageMap::IN_USE)
                canExtend = false;
        }

        if (canExtend)
        {
            um->markBlocks(inode->blockAddress + oldBlocks, inode->blockAddress + newBlocks, false);
        }
        else
        {
            int freeBlocksStart = um->getFreeBlocks(newSize);
            if (freeBlocksStart == -1)
                return -1;

            um->markBlocks(freeBlocksStart, freeBlocksStart + newBlocks, false);
            memcpy(getShmAddr((unsigned int)freeBlocksStart), getShmAddr(inode->blockAddress), inode->nodeSize);
            um->markBlocks(inode->blockAddress, inode->blockAddress + oldBlocks, true);
            inode->blockAddress = (unsigned int)freeBlocksStart;
        }

        inode->modificationDate = time(0);
        inode->nodeSize = newSize;

        return 0;


        //TO-DO -> update modification time and access date
    }
}

DiskOperations::DiskOperations(const unsigned int maxInodesCount, const unsigned int blockSize, const unsigned int fsSize):
        maxInodesCount(maxInodesCount), blockSize(blockSize), fsSize(fsSize)
{

}

unsigned char* DiskOperations::getShmAddr(unsigned int blockIndex)
{
    return shmaddr + blockIndex * blockSize;
}

Inode* DiskOperations::getInodeById(unsigned int id)
{
    for (int i = 0; i < ds->inodesCount; ++i)
    {
        if (inodeList->inodesArray[i].inodeId == id)
            return (Inode*)getShmAddr(inodeList->inodesArray[i].inodeAddress);
    }
    return nullptr;
}

int DiskOperations::initShm()
{
    // TODO: create shm file
    key = ftok("/dev/shm/miau", IPC_PRIVATE);
    if (key == -1)
        return -1;

    shmid = shmget(key, fsSize, 0777 | IPC_CREAT);
    if (shmid == -1)
        return -1;



    shmaddr = (unsigned char*)shmat(shmid, (void*)0,0);

    return 0;
}

int DiskOperations::initDiskStructures()
{
    ds = (DiskDescriptor*)shmaddr;
    ds->blocksCount = fsSize / blockSize;
    ds->maxInodesCount = maxInodesCount;
    ds->inodesCount = 0;
    ds->volumeId = 0;
    ds->freeInodeId = 0;
    strcpy(ds->volumeName, "aaa");

    unsigned char* bitmap = shmaddr + blockSize;
    um = new UsageMap(ds->blocksCount, bitmap);

    InodeListEntry* inodeTableAddr = (InodeListEntry*) shmaddr + blockSize + ceil(ds->blocksCount, blockSize) * blockSize;

    inodeList = new InodeList(ds, inodeTableAddr);

    int numBlocks = 1 + ceil(ds->blocksCount, blockSize) + ceil(sizeof(InodeListEntry) * ds->maxInodesCount, blockSize);
    um->markBlocks(0, numBlocks, false);

    return 0;
}

int DiskOperations::initRoot()
{
    unsigned int inodeBlockAddress = um->getFreeBlocks(1);
    um->markBlocks(inodeBlockAddress, inodeBlockAddress+1, false);

    unsigned int inodeDataBlockAddress = um->getFreeBlocks(1);
    um->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress+1, false);

    Inode* inode = (Inode*)(getShmAddr(inodeBlockAddress));
    Directory* directory = (Directory*)(getShmAddr(inodeDataBlockAddress));

    inode->id = ds->freeInodeId++;
    inode->fileType = Inode::IT_DIRECTORY;
    inode->permissions = Inode::PERM_R | Inode::PERM_W;
    inode->modificationDate = time(0);
    inode->accessDate = time(0);
    inode->creationDate = time(0);
    inode->deletionDate = NULL;
    inode->blockAddress = inodeDataBlockAddress;

    directory->init(inode->id, inode->id);

    inode->nodeSize = directory->getSize();

    InodeListEntry entry;
    entry.inodeAddress = inodeBlockAddress;
    entry.inodeId = inode->id;

    inodeList->addInodeEntry(entry);

    return 0;
}

ErrorResponse* DiskOperations::newErrorResponse(int errnum) {
    ErrorResponse* errorResponse = new ErrorResponse;
    errorResponse -> setErrno(errnum);
    return errorResponse;
}

DiskOperations::~DiskOperations()
{
    shmdt(shmaddr);

    shmctl(shmid, IPC_RMID, NULL);
}

Packet* DiskOperations::mkdir(char *path, int mode) //TODO - add sync (include semaphores, implement them)
{
    size_t pathLength = strlen(path);
    if (pathLength < 1)
        return newErrorResponse(ENOENT);
    else if (pathLength == 1)
    {
        if (path[0] != '/')
            return newErrorResponse(ENOTDIR);
        else
            return newErrorResponse(EEXIST);
    }
    else if (path[0] != '/')
        return newErrorResponse(ENOTDIR);
    int startIndex = 1;
    int endIndex = 1;
    unsigned int parentInodeId = 0;
    while (endIndex < pathLength)
    {
        while (path[endIndex] != '/' && path[endIndex] != '\0')
            ++endIndex;
        char* folderName = new char[endIndex - startIndex + 1];
        folderName[endIndex - startIndex] = '\0';
        strncpy(folderName, path + startIndex, (size_t)endIndex - startIndex);
        Inode* parentInode = getInodeById(parentInodeId);
        Directory* parentDir = (Directory*)getShmAddr(parentInode->blockAddress);
        InodeDirectoryEntry* dirList = parentDir->getInodesArray();
        int i = 0;
        while (i < parentDir->inodesCount)
        {
            if (strcmp(dirList[i].inodeName, folderName) == 0)
            {
                Inode* currentInode = getInodeById(dirList[i].inodeId);
                if (currentInode->fileType == Inode::IT_FILE)
                    return newErrorResponse(ENOTDIR);
                else if (path[endIndex] == '\0')
                    return newErrorResponse(EEXIST);
                else
                {
                    ++endIndex;
                    startIndex = endIndex;
                    parentInodeId = currentInode->id;
                }
            }
            ++i;
        }
        if (i == parentDir->inodesCount)
        {
            if ((parentInode->permissions & Inode::PERM_W) == 0)
                return newErrorResponse(EACCES);

            int inodeBlockAddress = um->getFreeBlocks(1);
            if (inodeBlockAddress == -1)
                return newErrorResponse(ENOSPC);
            um->markBlocks(inodeBlockAddress, inodeBlockAddress+1, false);

            int inodeDataBlockAddress = um->getFreeBlocks(1);
            if (inodeDataBlockAddress == -1)
            {
                um->markBlocks(inodeBlockAddress, inodeBlockAddress+1, true);
                return newErrorResponse(ENOSPC);
            }
            um->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress+1, false);

            Inode* inode = (Inode*)(getShmAddr(inodeBlockAddress));
            Directory* directory = (Directory*)(getShmAddr(inodeDataBlockAddress));

            inode->id = ds->freeInodeId++;
            inode->fileType = Inode::IT_DIRECTORY;
            inode->permissions = mode & (Inode::PERM_W | Inode::PERM_R);
            inode->modificationDate = time(0);
            inode->accessDate = time(0);
            inode->creationDate = time(0);
            inode->deletionDate = NULL;
            inode->blockAddress = inodeDataBlockAddress;

            directory->init(parentInodeId, inode->id);

            inode->nodeSize = directory->getSize();

            InodeListEntry entry;
            entry.inodeAddress = inodeBlockAddress;
            entry.inodeId = inode->id;

            inodeList->addInodeEntry(entry);

            reallocate(parentInode, parentDir->getSize() + sizeof(InodeDirectoryEntry));
            parentDir->addEntry(InodeDirectoryEntry(inode->id, folderName));
            return new OKResponse;
        }
    }
    return nullptr;
}
