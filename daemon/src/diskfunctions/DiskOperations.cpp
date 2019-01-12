#include <sys/ipc.h>
#include "DiskOperations.h"
#include "../management/Directory.h"
#include <errno.h>
#include <cstdio>
#include <sys/shm.h>
#include <cstring>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>


using namespace simplefs;

unsigned int ceil(unsigned int up, unsigned int down)
{
    unsigned int result = up / down;
    if (up % down != 0)
        ++result;
    return result;
}

unsigned char* DiskOperations::reallocate(Inode* inode, unsigned int newSize)
{
    unsigned int oldBlocks = ceil(inode->nodeSize, blockSize);
    unsigned int newBlocks = ceil(newSize, blockSize);
    unsigned char* toRet = nullptr;
    if (oldBlocks == newBlocks)
    {
        inode->modificationDate = time(0);
        inode->nodeSize = newSize;
        return getShmAddr(inode->blockAddress);
    }
    else if (oldBlocks > newBlocks)
    {
        inode->modificationDate = time(0);
        inode->nodeSize = newSize;
        um->markBlocks(inode->blockAddress + newBlocks, inode->blockAddress + oldBlocks, true);
        return getShmAddr(inode->blockAddress);
    }
    else
    {
        bool canExtend = true;
        for (unsigned int x = inode->blockAddress + oldBlocks; x < inode->blockAddress + newBlocks; ++x)
        {
            if (um->blocks[x] == UsageMap::IN_USE)
            {
                canExtend = false;
                break;
            }
        }

        if (canExtend)
        {
            um->markBlocks(inode->blockAddress + oldBlocks, inode->blockAddress + newBlocks, false);
            toRet = getShmAddr(inode->blockAddress);
        }
        else
        {
            int freeBlocksStart = um->getFreeBlocks(newBlocks);
            if (freeBlocksStart == -1)
                return nullptr;

            toRet = getShmAddr((unsigned int)freeBlocksStart);
            um->markBlocks(freeBlocksStart, freeBlocksStart + newBlocks, false);
            memcpy(toRet, getShmAddr(inode->blockAddress), inode->nodeSize);
            um->markBlocks(inode->blockAddress, inode->blockAddress + oldBlocks, true);
            inode->blockAddress = (unsigned int)freeBlocksStart;
        }

        inode->modificationDate = time(0);
        inode->nodeSize = newSize;

        return toRet;
    }
}

DiskOperations::DiskOperations(const char* volumeName, const unsigned int volumeId, const unsigned int maxInodesCount,
        const unsigned int blockSize, const unsigned int fsSize):
        volumeName(volumeName), volumeId(volumeId), maxInodesCount(maxInodesCount), blockSize(blockSize), fsSize(fsSize)
{
    sem_init(&inodeOpSemaphore, 0, 1);
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
    char* shmFilePath = new char[10 + strlen(volumeName)];
    strcat(shmFilePath, "/dev/shm/");
    strcat(shmFilePath, volumeName);
    int createdShmFd = creat(shmFilePath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (createdShmFd == -1)
    {
        delete[] shmFilePath;
        return -1;
    }
    close(createdShmFd);
    key = ftok(shmFilePath, IPC_PRIVATE);
    delete[] shmFilePath;
    if (key == -1)
        return -1;

    shmid = shmget(key, fsSize, 0777 | IPC_CREAT);
    if (shmid == -1)
        return -1;

    shmaddr = (unsigned char*)shmat(shmid, nullptr, 0);

    return 0;
}

int DiskOperations::initDiskStructures()
{
    ds = (DiskDescriptor*)shmaddr;
    ds->blocksCount = fsSize / blockSize;
    ds->maxInodesCount = maxInodesCount;
    ds->inodesCount = 0;
    ds->volumeId = volumeId;
    ds->freeInodeId = 0;
    strcpy(ds->volumeName, volumeName);

    unsigned char* bitmap = shmaddr + blockSize;
    um = new UsageMap(ds->blocksCount, bitmap);

    InodeListEntry* inodeTableAddr = (InodeListEntry*) (shmaddr + ceil(sizeof(DiskDescriptor), blockSize) * blockSize
            + ceil(ds->blocksCount, blockSize) * blockSize);

    inodeList = new InodeList(ds, inodeTableAddr);

    int numBlocks = ceil(sizeof(DiskDescriptor), blockSize)
            + ceil(ds->blocksCount, blockSize) + ceil(sizeof(InodeListEntry) * ds->maxInodesCount, blockSize);
    um->markBlocks(0, numBlocks, false);

    return 0;
}

int DiskOperations::initRoot()
{
    unsigned int inodeBlocks = ceil(sizeof(Inode), blockSize);
    int inodeBlockAddress = um->getFreeBlocks(inodeBlocks);
    if (inodeBlockAddress == -1)
        return -1;
    um->markBlocks(inodeBlockAddress, inodeBlockAddress + inodeBlocks, false);

    unsigned int inodeDataBlocks = ceil(2 * sizeof(InodeDirectoryEntry) + sizeof(Directory), blockSize);
    int inodeDataBlockAddress = um->getFreeBlocks(inodeDataBlocks);
    if (inodeDataBlockAddress == -1)
        return -1;
    um->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress + inodeDataBlocks, false);

    createNewDirEntry(0, inodeBlockAddress, inodeDataBlockAddress, Inode::PERM_R | Inode::PERM_W);

    return 0;
}

ErrorResponse* DiskOperations::newErrorResponse(int errnum) {
    ErrorResponse* errorResponse = new ErrorResponse;
    errorResponse -> setErrno(errnum);
    return errorResponse;
}

void DiskOperations::createNewDirEntry(unsigned int parentInodeId, int inodeBlockAddress, int inodeDataBlockAddress, int mode)
{
    Inode* inode = (Inode*)(getShmAddr((unsigned int)inodeBlockAddress));
    Directory* directory = (Directory*)(getShmAddr((unsigned int)inodeDataBlockAddress));

    inode->id = ds->freeInodeId++;
    inode->fileType = Inode::IT_DIRECTORY;
    inode->permissions = mode & (Inode::PERM_W | Inode::PERM_R);
    inode->modificationDate = time(0);
    inode->accessDate = time(0);
    inode->creationDate = time(0);
    inode->deletionDate = 0;
    inode->blockAddress = (unsigned int)inodeDataBlockAddress;

    directory->init(parentInodeId, inode->id);

    inode->nodeSize = directory->getSize();

    InodeListEntry entry{};
    entry.inodeAddress = (unsigned int)inodeBlockAddress;
    entry.inodeId = inode->id;

    inodeList->addInodeEntry(entry);
}

DiskOperations::~DiskOperations()
{
    sem_destroy(&inodeOpSemaphore);

    shmdt(shmaddr);

    shmctl(shmid, IPC_RMID, nullptr);
}

Packet* DiskOperations::mkdir(const char *path, int mode) //TODO - add sync (include semaphores, implement them)
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
    sem_wait(&inodeOpSemaphore);
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
                {
                    sem_post(&inodeOpSemaphore);
                    delete[] folderName;
                    return newErrorResponse(ENOTDIR);
                }
                else if (path[endIndex] == '\0')
                {
                    sem_post(&inodeOpSemaphore);
                    delete[] folderName;
                    return newErrorResponse(EEXIST);
                }
                else
                {
                    ++endIndex;
                    startIndex = endIndex;
                    parentInodeId = currentInode->id;
                    break;
                }
            }

            ++i;
        }
        if (i == parentDir->inodesCount)
        {
            if ((parentInode->permissions & Inode::PERM_W) == 0)
            {
                sem_post(&inodeOpSemaphore);
                delete[] folderName;
                return newErrorResponse(EACCES);
            }

            if (strcmp(folderName, "..") == 0 || strcmp(folderName, ".") == 0)
            {
                sem_post(&inodeOpSemaphore);
                delete[] folderName;
                return newErrorResponse(EEXIST);
            }
            unsigned int inodeBlocks = ceil(sizeof(Inode), blockSize);
            int inodeBlockAddress = um->getFreeBlocks(inodeBlocks);
            if (inodeBlockAddress == -1)
            {
                sem_post(&inodeOpSemaphore);
                delete[] folderName;
                return newErrorResponse(ENOSPC);
            }
            um->markBlocks(inodeBlockAddress, inodeBlockAddress+inodeBlocks, false);

            unsigned int inodeDataBlocks = ceil(2 * sizeof(InodeDirectoryEntry) + sizeof(Directory), blockSize);
            int inodeDataBlockAddress = um->getFreeBlocks(inodeDataBlocks);
            if (inodeDataBlockAddress == -1)
            {
                um->markBlocks(inodeBlockAddress, inodeBlockAddress+inodeBlocks, true);
                sem_post(&inodeOpSemaphore);
                delete[] folderName;
                return newErrorResponse(ENOSPC);
            }
            um->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress+inodeDataBlocks, false);

            createNewDirEntry(parentInodeId, inodeBlockAddress, inodeDataBlockAddress, mode);

            Inode* inode = (Inode*)(getShmAddr((unsigned int)inodeBlockAddress));
            parentDir = (Directory *)reallocate(parentInode, parentDir->getSize() + sizeof(InodeDirectoryEntry));
            parentDir->addEntry(InodeDirectoryEntry(inode->id, folderName));
            sem_post(&inodeOpSemaphore);
            delete[] folderName;
            return new OKResponse;
        }
        delete[] folderName;
    }
    sem_post(&inodeOpSemaphore);
    return nullptr;
}

void DiskOperations::printUsageMap()
{
    for (int x = 0; x < um->size; ++x)
    {
        printf("%d ", um->blocks[x]);
    }
    printf("\n");
}

void DiskOperations::printInodeParams(int i)
{
    InodeListEntry id = inodeList->inodesArray[i];
    printf("%d %d ", id.inodeId, id.inodeAddress);
    Inode* in = getInodeById(id.inodeId);
    printf("%d %d\n", in->blockAddress, in->permissions);
}

void DiskOperations::printInodes()
{
    for (int i = 0; i < ds->inodesCount; ++i)
        printInodeParams(i);
}
