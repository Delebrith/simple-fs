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
#include <algorithm>    // std::max


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
        const unsigned int blockSize, const unsigned int fsSize, FileDescriptorTable* fdTable):
        volumeName(volumeName), volumeId(volumeId), maxInodesCount(maxInodesCount), blockSize(blockSize), fsSize(fsSize), fdTable(fdTable)
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

    int numBlocks = ceil(sizeof(DiskDescriptor), blockSize)
                    + ceil(ds->blocksCount, blockSize) + ceil(sizeof(InodeListEntry) * ds->maxInodesCount, blockSize);

    if (numBlocks > ds->blocksCount)
        return -1;

    unsigned char* bitmap = shmaddr + blockSize;
    um = new UsageMap(ds->blocksCount, bitmap);

    InodeListEntry* inodeTableAddr = (InodeListEntry*) (shmaddr + ceil(sizeof(DiskDescriptor), blockSize) * blockSize
            + ceil(ds->blocksCount, blockSize) * blockSize);

    inodeList = new InodeList(ds, inodeTableAddr);
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

    createNewInodeEntry(0, inodeBlockAddress, inodeDataBlockAddress, Inode::PERM_R | Inode::PERM_W, Inode::IT_DIRECTORY);

    return 0;
}

ErrorResponse* DiskOperations::newErrorResponse(int errnum) {
    ErrorResponse* errorResponse = new ErrorResponse;
    errorResponse -> setErrno(errnum);
    return errorResponse;
}

Inode* DiskOperations::createNewInodeEntry(unsigned int parentInodeId, int inodeBlockAddress, int inodeDataBlockAddress, int mode, int inodeFileType)
{
    Inode* inode = (Inode*)(getShmAddr((unsigned int)inodeBlockAddress));

    inode->id = ds->freeInodeId++;
    inode->fileType = inodeFileType; //Inode::IT_DIRECTORY;
    inode->permissions = mode & (Inode::PERM_W | Inode::PERM_R | Inode::PERM_X);
    inode->modificationDate = time(0);
    inode->accessDate = time(0);
    inode->creationDate = time(0);
    inode->deletionDate = 0;
    inode->blockAddress = (unsigned int)inodeDataBlockAddress;

    inode->nodeSize = 0;

    if(inodeFileType == Inode::IT_DIRECTORY)
    {
        Directory* directory = (Directory*)(getShmAddr((unsigned int)inodeDataBlockAddress));
        directory->init(parentInodeId, inode->id);
        inode->nodeSize = directory->getSize();
    }

    InodeListEntry entry{};
    entry.inodeAddress = (unsigned int)inodeBlockAddress;
    entry.inodeId = inode->id;

    inodeList->addInodeEntry(entry);

    return inode;
}

DiskOperations::~DiskOperations()
{
    delete um;
    delete inodeList;
    sem_destroy(&inodeOpSemaphore);

    shmdt(shmaddr);

    shmctl(shmid, IPC_RMID, nullptr);
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
Inode* DiskOperations::dirNavigate(const char* path) // just for testing
{
    size_t pathLength = strlen(path);
    if (pathLength < 1)
    {
        errno = ENOENT;
        return nullptr;
    }
    else if (pathLength == 1)
    {
        if (path[0] != '/')
        {
            errno = ENOTDIR;
            return nullptr;
        }
        else
        {
            errno = EEXIST;
            sem_wait(&inodeOpSemaphore);
                Inode* inodeRet = getInodeById(0);
            sem_post(&inodeOpSemaphore);
            return inodeRet;

        }
    }
    else if (path[0] != '/')
    {
        errno = ENOTDIR;
        return nullptr;
    }
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

        if ((parentInode->permissions & Inode::PERM_R) == 0)
        {
            sem_post(&inodeOpSemaphore);
            delete[] folderName;

            errno = EACCES;
            return nullptr;
        }

        Directory* parentDir = (Directory*)getShmAddr(parentInode->blockAddress);
        InodeDirectoryEntry* dirList = parentDir->getInodesArray();
        int i = 0;
        while (i < parentDir->inodesCount)
        {
            if (strcmp(dirList[i].inodeName, folderName) == 0)
            {
                Inode* currentInode = getInodeById(dirList[i].inodeId);

                ++endIndex;
                startIndex = endIndex;
                parentInodeId = currentInode->id;
                break;
            }

            ++i;
        }
        if(i == parentDir->inodesCount) // file not found
        {
            sem_post(&inodeOpSemaphore);
            delete[] folderName;

            errno = ENOENT;
            return nullptr;
        }
        sem_post(&inodeOpSemaphore);
        delete[] folderName;
        Inode* inode = getInodeById(parentInodeId);
        return inode;

    }
    sem_post(&inodeOpSemaphore);

    errno = ENOSYS;
    return nullptr;
}


Inode* DiskOperations::createInode(const char *path, int mode, int inodeFileType) //TODO - add sync (include semaphores, implement them)
{
    size_t pathLength = strlen(path);
    if (pathLength < 1)
    {
        errno = ENOENT;
        return nullptr;
    }

    else if (pathLength == 1)
    {
        if (path[0] != '/')
        {
            errno = ENOTDIR;
            return nullptr;
        }
        else
        {
            errno = EEXIST;
            return nullptr;
        }
    }
    else if (path[0] != '/')
    {
        errno = ENOTDIR;
        return nullptr;
    }
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

        if ((parentInode->permissions & Inode::PERM_R) == 0)
        {
            sem_post(&inodeOpSemaphore);
            delete[] folderName;

            errno = EACCES;
            return nullptr;
        }

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

                    errno = ENOTDIR;
                    return nullptr;
                }
                else if (path[endIndex] == '\0')
                {
                    sem_post(&inodeOpSemaphore);
                    delete[] folderName;

                    errno = EEXIST;
                    return nullptr;
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

                errno = EACCES;
                return nullptr;
            }

            if (strcmp(folderName, "..") == 0 || strcmp(folderName, ".") == 0)
            {
                sem_post(&inodeOpSemaphore);
                delete[] folderName;

                errno = EEXIST;
                return nullptr;
            }
            unsigned int inodeBlocks = ceil(sizeof(Inode), blockSize);
            int inodeBlockAddress = um->getFreeBlocks(inodeBlocks);
            if (inodeBlockAddress == -1)
            {
                sem_post(&inodeOpSemaphore);
                delete[] folderName;

                errno = ENOSPC;
                return nullptr;

            }
            um->markBlocks(inodeBlockAddress, inodeBlockAddress+inodeBlocks, false);

            unsigned int inodeDataBlocks = 1;

            if(inodeFileType == Inode::IT_DIRECTORY)
            {
                inodeDataBlocks = ceil(2 * sizeof(InodeDirectoryEntry) + sizeof(Directory), blockSize);
            }

            int inodeDataBlockAddress = um->getFreeBlocks(inodeDataBlocks);
            if (inodeDataBlockAddress == -1)
            {
                um->markBlocks(inodeBlockAddress, inodeBlockAddress+inodeBlocks, true);
                sem_post(&inodeOpSemaphore);
                delete[] folderName;

                errno = ENOSPC;
                return nullptr;

            }
            um->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress+inodeDataBlocks, false);


            createNewInodeEntry(parentInodeId, inodeBlockAddress, inodeDataBlockAddress, mode, inodeFileType);


            Inode* inode = (Inode*)(getShmAddr((unsigned int)inodeBlockAddress));
            parentDir = (Directory *)reallocate(parentInode, parentDir->getSize() + sizeof(InodeDirectoryEntry));
            parentDir->addEntry(InodeDirectoryEntry(inode->id, folderName));

            sem_post(&inodeOpSemaphore);
            delete[] folderName;
            return inode;
        }
        delete[] folderName;
    }
    sem_post(&inodeOpSemaphore);

    errno = ENOSYS;
    return nullptr;
}
Packet* DiskOperations::mkdir(const char* path, int permissions)
{
    Inode* ret = createInode(path, permissions, Inode::IT_DIRECTORY);
    if(ret == nullptr)
    {
        return newErrorResponse(errno);
    }
    return new OKResponse();
}

Packet *DiskOperations::lseek(FileDescriptor *fd, int offset, int whence)
{
    if(fd == nullptr) return newErrorResponse(EBADF);
    sem_wait(&fd->fdSemaphore);
    /*if (fd->inode->fileType == Inode::IT_DIRECTORY)
    {
        sem_post(&fd->fdSemaphore);
        return newErrorResponse(EISDIR);
    }*/
    if (whence == SEEK_SET)
    {
        if (offset < 0 || offset > fd->inode->nodeSize)
        {
            sem_post(&fd->fdSemaphore);
            return newErrorResponse(EINVAL);
        }
        fd->position = (unsigned long long)offset;
    }
    else if (whence == SEEK_CUR || whence == SEEK_END)
    {
        unsigned long long newPosition;
        if (whence == SEEK_CUR)
            newPosition = fd->position + offset;
        else
            newPosition = fd->inode->nodeSize + offset;
        if (newPosition < 0 || newPosition > fd->inode->nodeSize)
        {
            sem_post(&fd->fdSemaphore);
            return newErrorResponse(EINVAL);
        }
        fd->position = newPosition;
    }
    else
    {
        sem_post(&fd->fdSemaphore);
        return newErrorResponse(EINVAL);
    }
    sem_post(&fd->fdSemaphore);
    return new OKResponse;
}
Packet* DiskOperations::create(const char* path, int mode, int pid)
{
    // mode is not used here.
    return open(path, O_CREAT | O_WRONLY, pid);
}
Packet* DiskOperations::open(const char* path, int flags, int pid)
{
    return openUsingFileDescriptorFlags(path, linuxIntoFileDescriptorFlags(flags), pid);
}
Packet* DiskOperations::openUsingFileDescriptorFlags(const char* path, int flags, int pid)
{
    Inode* inodeToOpen = dirNavigate(path);
    if(inodeToOpen == nullptr) // eventually creating a file
    {
        if(errno == ENOENT)
        {
            if(flags & FileDescriptor::M_CREATE)
            {
                unsigned int permissions = 0;
                if(flags & FileDescriptor::M_CREATE_PERM_R) permissions |= Inode::PERM_R;
                if(flags & FileDescriptor::M_CREATE_PERM_W) permissions |= Inode::PERM_W;
                if(flags & FileDescriptor::M_CREATE_PERM_X) permissions |= Inode::PERM_X;
                inodeToOpen = createInode(path, permissions, Inode::IT_FILE);
                if(inodeToOpen == nullptr)
                {
                    return newErrorResponse(errno);
                }
            }
            else
            {
                return newErrorResponse(errno);
            }
        }
        else
        {
            return newErrorResponse(errno);
        }
    }

    if(inodeToOpen->fileType == Inode::IT_DIRECTORY && flags != FileDescriptor::M_READ)
    {
        return newErrorResponse(EISDIR);
    }

    if((flags & (FileDescriptor::M_READ | FileDescriptor::M_WRITE)) == (FileDescriptor::M_READ | FileDescriptor::M_WRITE)) // RW Access
    {
        if(fdTable->inodeStatusMap.OpenForReadWrite(inodeToOpen))
        {
            return newErrorResponse(errno);
        }
    }
    if(flags & FileDescriptor::M_READ) // Ronly Access
    {
        if(fdTable->inodeStatusMap.OpenForReading(inodeToOpen))
        {
            return newErrorResponse(errno);
        }
    }
    if(flags & FileDescriptor::M_WRITE) // Wonly Access
    {
        if(fdTable->inodeStatusMap.OpenForWriting(inodeToOpen))
        {
            return newErrorResponse(errno);
        }
    }

    FileDescriptor* fd = fdTable->CreateDescriptor(pid, inodeToOpen, flags);
    FDResponse* fdres = new FDResponse();
    fdres->setFD(fd->number);
    return fdres;
}
Packet* DiskOperations::unlink(const char* path)
{
    return nullptr;
}
Packet* DiskOperations::read(FileDescriptor* fd, int len)
{
    if(fd == nullptr) return newErrorResponse(EBADF);
    sem_wait(&fd->fdSemaphore);

    int bytesAvailable = std::max((int32_t)0,(int32_t)fd->inode->nodeSize-(int32_t)fd->position);
    int bytesRead = std::min(bytesAvailable, len);

    printf("\n>READ: %d bytes from %d\n\n", bytesRead, fd->position);

    ShmemPtr shmemPtr;
    shmemPtr.shmid = shmid;
    shmemPtr.offset = (unsigned int)(fd->inode->blockAddress*blockSize + fd->position);
    shmemPtr.size = (unsigned int)bytesRead;

    ShmemPtrResponse* shmemPtrResponse = new ShmemPtrResponse;
    shmemPtrResponse->setPtr(shmemPtr);

    fd->position += bytesRead;

    sem_post(&fd->fdSemaphore);
    return shmemPtrResponse;
}
Packet* DiskOperations::write(FileDescriptor* fd, int len)
{
    return nullptr;
}
Packet* DiskOperations::chmod(const char* path, int mode)
{
    Inode* inodeToOpen = dirNavigate(path);
    if(inodeToOpen == nullptr) // error
    {
        return newErrorResponse(errno);
    }

    if(fdTable->inodeStatusMap.InodeStatus(inodeToOpen) != 0)
    {
        return newErrorResponse(EACCES);
    }

    sem_wait(&inodeOpSemaphore);

    inodeToOpen->permissions = 0;

    if(mode & S_IROTH) inodeToOpen->permissions |= Inode::PERM_R;
    if(mode & S_IWOTH) inodeToOpen->permissions |= Inode::PERM_W;
    if(mode & S_IXOTH) inodeToOpen->permissions |= Inode::PERM_X;

    sem_post(&inodeOpSemaphore);

    return new OKResponse();

}

int DiskOperations::linuxIntoFileDescriptorFlags(int flags)
{
    int outmode = 0;

    if(flags & O_WRONLY) outmode |= FileDescriptor::M_WRITE;
    else if(flags & O_RDWR) outmode |= FileDescriptor::M_WRITE | FileDescriptor::M_READ;
    else outmode |= FileDescriptor::M_READ;

    if(flags & O_CREAT)
    {
        // let's permissions just be RW
        outmode |= FileDescriptor::M_CREATE;
        outmode |= FileDescriptor::M_CREATE_PERM_R;
        outmode |= FileDescriptor::M_CREATE_PERM_W;
    }

    return outmode;
}
