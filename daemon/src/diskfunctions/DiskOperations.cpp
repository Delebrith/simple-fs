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

inline int ceil(int value, int divisor)
{
    return value / divisor + (value % divisor != 0);
}

unsigned char* DiskOperations::reallocate(Inode* inode, unsigned int newSize)
{
    unsigned int oldBlocks = ceil(inode->nodeSize, blockSize);
    unsigned int newBlocks = ceil(newSize, blockSize);
    unsigned char* toRet = nullptr;
    if (oldBlocks == newBlocks)
    {
        inode->nodeSize = newSize;
        return getShmAddr(inode->blockAddress);
    }
    else if (oldBlocks > newBlocks)
    {
        inode->nodeSize = newSize;
        um->markBlocks(inode->blockAddress + newBlocks, inode->blockAddress + oldBlocks, true, false);
        return getShmAddr(inode->blockAddress);
    }
    else
    {
        um->lock();

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
            um->markBlocks(inode->blockAddress + oldBlocks, inode->blockAddress + newBlocks, false, false);
            toRet = getShmAddr(inode->blockAddress);
        }
        else
        {
            int freeBlocksStart = um->getFreeBlocks(newBlocks, false);
            if (freeBlocksStart != -1)
            {
                toRet = getShmAddr((unsigned int)freeBlocksStart);
                um->markBlocks(freeBlocksStart, freeBlocksStart + newBlocks, false, false);
                memcpy(toRet, getShmAddr(inode->blockAddress), inode->nodeSize);
                um->markBlocks(inode->blockAddress, inode->blockAddress + oldBlocks, true, false);
                inode->blockAddress = (unsigned int)freeBlocksStart;
            }
        }

        inode->nodeSize = newSize;

        um->unlock();
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
            return inodeList->inodesArray[i].inodeAddress;
    }
    return nullptr;
}

Inode *DiskOperations::getFreeInodeAddr() {
    return inodes + ds->inodesCount;
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
                    + ceil(ds->blocksCount, blockSize) + ceil(sizeof(InodeListEntry) * ds->maxInodesCount, blockSize)
                    + ceil(sizeof(Inode) * ds->maxInodesCount, blockSize);

    if (numBlocks > ds->blocksCount)
        return -1;

    unsigned char* bitmap = shmaddr + blockSize;
    um = new UsageMap(ds->blocksCount, bitmap);

    InodeListEntry* inodeTableAddr = (InodeListEntry*) (shmaddr + ceil(sizeof(DiskDescriptor), blockSize) * blockSize
            + ceil(ds->blocksCount, blockSize) * blockSize);

    inodeList = new InodeList(ds, inodeTableAddr);

    inodes = (Inode*)(shmaddr + ceil(sizeof(DiskDescriptor), blockSize) * blockSize
                                 + ceil(ds->blocksCount, blockSize) * blockSize
                                 + ceil(sizeof(InodeListEntry) * ds->maxInodesCount, blockSize) * blockSize);
    um->markBlocks(0, numBlocks, false, true);

    return 0;
}

int DiskOperations::initRoot()
{
    unsigned int inodeDataBlocks = ceil(2 * sizeof(InodeDirectoryEntry) + sizeof(Directory), blockSize);
    int inodeDataBlockAddress = um->getFreeBlocks(inodeDataBlocks, false);
    if (inodeDataBlockAddress == -1)
        return -1;
    um->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress + inodeDataBlocks, false, false);

    createNewInodeEntry(0, getFreeInodeAddr(), inodeDataBlockAddress, Inode::PERM_R | Inode::PERM_W, Inode::IT_DIRECTORY);

    return 0;
}

Inode* DiskOperations::createNewInodeEntry(unsigned int parentInodeId, Inode* inodeAddress, int inodeDataBlockAddress, int mode, int inodeFileType)
{
    Inode* inode = inodeAddress;

    inode->id = ds->freeInodeId++;
    inode->fileType = inodeFileType;
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
    entry.inodeAddress = inodeAddress;
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
    printf("%d %p ", id.inodeId, id.inodeAddress);
    Inode* in = getInodeById(id.inodeId);
    printf("%d %d\n", in->blockAddress, in->permissions);
}

void DiskOperations::printInodes()
{
    for (int i = 0; i < ds->inodesCount; ++i)
        printInodeParams(i);
}

int getLastMemberLen(const char* path, int pathLen)
{
    int len = 0;

    for (const char* ch = path + pathLen; ch != path; ++len)
    {
        --ch;
        if (*ch == '/')
            return len;
    }

    return len;
}

Inode* DiskOperations::getMember(Inode* parentDirInode, const char* name, int nameLen, int& error)
{
    if (!(parentDirInode->permissions & Inode::PERM_R))
    {
        error = EPERM;
    }

    Directory* dir = (Directory*)getShmAddr(parentDirInode->blockAddress);

    int len = nameLen;
    if (name[nameLen - 1] == '/')
        --len;

    for (int i = 0; i < dir->inodesCount; ++i)
        if (strncmp(dir->getInodesArray()[i].inodeName, name, len) == 0 && dir->getInodesArray()[i].inodeName[len] == 0)
            return getInodeById(dir->getInodesArray()[i].inodeId);

    error = ENOENT;
    return nullptr;
}

Inode* DiskOperations::getParent(const char* path, int pathLen, int &error)
{
    int lastMemberLen = getLastMemberLen(path, pathLen);

    if (pathLen == 1 && path[0] == '/')
        return getInodeById(0);

    if (lastMemberLen == pathLen)
    {
        error = ENOTDIR;
        return nullptr;
    }

    Inode* parent = getParent(path, pathLen - lastMemberLen, error);

    if (parent == nullptr)
        return nullptr;

    if (parent->fileType != Inode::IT_DIRECTORY)
    {
        error = ENOTDIR;
        return nullptr;
    }

    return getMember(parent, path + pathLen - lastMemberLen, lastMemberLen, error);
}

Inode* DiskOperations::dirNavigate(const char* path, int& error)
{
    size_t pathLen = strlen(path);

    Inode* parent = getParent(path, pathLen, error);

    if (parent == nullptr)
        return nullptr;
    if (pathLen == 1)
        return parent;

    int lastMemberLen = getLastMemberLen(path, pathLen);
    return getMember(parent, path + pathLen - lastMemberLen, lastMemberLen, error);
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
                    delete[] folderName;

                    errno = ENOTDIR;
                    return nullptr;
                }
                else if (path[endIndex] == '\0')
                {
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
                delete[] folderName;

                errno = EACCES;
                return nullptr;
            }

            if (strcmp(folderName, "..") == 0 || strcmp(folderName, ".") == 0)
            {
                delete[] folderName;

                errno = EEXIST;
                return nullptr;
            }
            unsigned int inodeDataBlocks = 1;

            if(inodeFileType == Inode::IT_DIRECTORY)
            {
                inodeDataBlocks = ceil(2 * sizeof(InodeDirectoryEntry) + sizeof(Directory), blockSize);
            }

            int inodeDataBlockAddress = um->getFreeBlocks(inodeDataBlocks, false);
            if (inodeDataBlockAddress == -1)
            {
                delete[] folderName;

                errno = ENOSPC;
                um->unlock();
                return nullptr;

            }
            um->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress+inodeDataBlocks, false, false);
            um->unlock();

            Inode* inode = getFreeInodeAddr();
            createNewInodeEntry(parentInodeId, inode, inodeDataBlockAddress, mode, inodeFileType);

            parentDir = (Directory *)reallocate(parentInode, parentDir->getSize() + sizeof(InodeDirectoryEntry));
            if (parentDir != nullptr)
            {
                parentDir->addEntry(InodeDirectoryEntry(inode->id, folderName));
                parentInode->modificationDate = time(0);
            }

            sem_post(&inodeOpSemaphore);
            delete[] folderName;
            return inode;
        }
        delete[] folderName;
    }

    errno = ENOSYS;
    return nullptr;
}
Packet* DiskOperations::mkdir(const char* path, int permissions)
{
    sem_wait(&inodeOpSemaphore);
    Inode* ret = createInode(path, permissions, Inode::IT_DIRECTORY);
    sem_post(&inodeOpSemaphore);
    if(ret == nullptr)
        return new ErrorResponse(errno);

    return new OKResponse();
}

Packet *DiskOperations::lseek(FileDescriptor *fd, int offset, int whence)
{
    if (whence == SEEK_SET)
    {
        if (offset < 0 || offset > fd->inode->nodeSize)
        {
            return new ErrorResponse(EINVAL);
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
            return new ErrorResponse(EINVAL);

        fd->position = newPosition;
    }
    else
        return new ErrorResponse(EINVAL);
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
    int error;

    sem_wait(&inodeOpSemaphore);
    Inode* inodeToOpen = dirNavigate(path, error);

    if(inodeToOpen == nullptr) // eventually creating a file
    {
        if(error == ENOENT && flags & FileDescriptor::M_CREATE)
        {
            unsigned int permissions = 0;
            if(flags & FileDescriptor::M_CREATE_PERM_R) permissions |= Inode::PERM_R;
            if(flags & FileDescriptor::M_CREATE_PERM_W) permissions |= Inode::PERM_W;
            if(flags & FileDescriptor::M_CREATE_PERM_X) permissions |= Inode::PERM_X;
            inodeToOpen = createInode(path, permissions, Inode::IT_FILE);

            if(inodeToOpen == nullptr)
            {
                sem_post(&inodeOpSemaphore);
                return new ErrorResponse(errno);
            }
        }
        else
        {
            sem_post(&inodeOpSemaphore);
            return new ErrorResponse(error);
        }
    }

    if(inodeToOpen->fileType == Inode::IT_DIRECTORY && flags != FileDescriptor::M_READ)
    {
        sem_post(&inodeOpSemaphore);
        return new ErrorResponse(EISDIR);
    }

    if((flags & (FileDescriptor::M_READ | FileDescriptor::M_WRITE)) == (FileDescriptor::M_READ | FileDescriptor::M_WRITE)) // RW Access
    {
        if(fdTable->inodeStatusMap.OpenForReadWrite(inodeToOpen))
        {
            sem_post(&inodeOpSemaphore);
            return new ErrorResponse(errno);
        }
    }
    if(flags & FileDescriptor::M_READ) // Ronly Access
    {
        if(fdTable->inodeStatusMap.OpenForReading(inodeToOpen))
        {
            sem_post(&inodeOpSemaphore);
            return new ErrorResponse(errno);
        }
    }
    if(flags & FileDescriptor::M_WRITE) // Wonly Access
    {
        if(fdTable->inodeStatusMap.OpenForWriting(inodeToOpen))
        {
            sem_post(&inodeOpSemaphore);
            return new ErrorResponse(errno);
        }
    }

    FileDescriptor* fd = fdTable->CreateDescriptor(pid, inodeToOpen, flags);
    FDResponse* fdres = new FDResponse();
    fdres->setFD(fd->number);

    sem_post(&inodeOpSemaphore);
    return fdres;
}

Packet* DiskOperations::unlink(const char* path)
{
    int len = strlen(path);

    if (len == 0 || path[0] != '/')
        return new ErrorResponse(ENOTDIR);

    if (len == 1)
        return new ErrorResponse(EBUSY);

    if (path[len - 1] == '.' && path[len - 2] == '/')
        return new ErrorResponse(EINVAL);

    if (len > 2 && path[len - 1] == '.' && path[len - 2] == '.' && path[len - 3] == '/')
        return new ErrorResponse(ENOTEMPTY);

    int error;

    sem_wait(&inodeOpSemaphore);
    Inode* parent = getParent(path, len, error);

    if (parent == nullptr)
    {
        sem_post(&inodeOpSemaphore);
        return new ErrorResponse(error);
    }

    if (!(parent->permissions & Inode::PERM_W))
    {
        sem_post(&inodeOpSemaphore);
        return new ErrorResponse(EPERM);
    }

    int removedLen = getLastMemberLen(path, len);
    Inode* removed = getMember(parent, path - removedLen, removedLen, error);

    if(removed == nullptr)
    {
        sem_post(&inodeOpSemaphore);
        return new ErrorResponse(error);
    }

    if(removed->fileType == Inode::IT_DIRECTORY && reinterpret_cast<Directory*>(getShmAddr(removed->blockAddress))->inodesCount > 2)
    {
        sem_post(&inodeOpSemaphore);
        return new ErrorResponse(ENOTEMPTY);
    }

    Directory* parentDir = (Directory*)getShmAddr(parent->blockAddress);

    parentDir->deleteEntry(removed->id);
    reallocate(parent, ceil(parentDir->getSize(), blockSize));

    sem_post(&inodeOpSemaphore);
    return new OKResponse;
}
Packet* DiskOperations::read(FileDescriptor* fd, int len)
{

    int bytesAvailable = std::max((int32_t)0,(int32_t)fd->inode->nodeSize-(int32_t)fd->position);
    int bytesRead = std::min(bytesAvailable, len);

    fd->inode->accessDate = time(0);

    ShmemPtr shmemPtr;
    shmemPtr.shmid = shmid;
    shmemPtr.offset = (unsigned int)(fd->inode->blockAddress*blockSize + fd->position);
    shmemPtr.size = (unsigned int)bytesRead;

    ShmemPtrResponse* shmemPtrResponse = new ShmemPtrResponse;
    shmemPtrResponse->setPtr(shmemPtr);

    fd->position += bytesRead;

    return shmemPtrResponse;
}

Packet* DiskOperations::write(FileDescriptor* fd, int len)
{
    if (fd->mode & FileDescriptor::M_WRITE == 0)
        return new ErrorResponse(EBADF);

    int newPos = fd->position + len;

    unsigned char* shmempos = reallocate(fd->inode, newPos);

    if (shmempos == nullptr)
        return new ErrorResponse(EFBIG);

    ShmemPtr ptr = ShmemPtr{.shmid=shmid, .offset=fd->inode->blockAddress*blockSize + fd->position, .size=len};
    ShmemPtrResponse* response = new ShmemPtrResponse;

    response->setPtr(ptr);

    fd->inode->modificationDate = time(0);
    fd->inode->accessDate = time(0);
    fd->position = newPos;

    return response;
}

Packet* DiskOperations::chmod(const char* path, int mode)
{
    int error;
    sem_wait(&inodeOpSemaphore);

    Inode* inodeToOpen = dirNavigate(path, error);
    if(inodeToOpen == nullptr) // error
    {
        sem_post(&inodeOpSemaphore);
        return new ErrorResponse(error);
    }

    if(fdTable->inodeStatusMap.InodeStatus(inodeToOpen) != 0)
    {
        sem_post(&inodeOpSemaphore);
        return new ErrorResponse(EACCES);
    }

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
