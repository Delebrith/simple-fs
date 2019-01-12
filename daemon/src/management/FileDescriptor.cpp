#include "FileDescriptor.h"
//
// Created by nkp123 on 12.01.19.
//

int InodeStatusMap::InodeStatus(Inode* inode)
{
    int ret;

    sem_wait(&statusMapSemaphore);
        if (statusMap.find(inode) == statusMap.end())
        {
            ret = 0;
        }
        else ret = statusMap[inode];
    sem_post(&statusMapSemaphore);

    return ret;
}
int InodeStatusMap::OpenForWriting(Inode* inode)
{
    int ret;

    sem_wait(&statusMapSemaphore);
        if (statusMap.find(inode) != statusMap.end())
        {
            errno = EACCES; //ENOSYS;
            ret = -1;
        }
        else
        {
            statusMap[inode] = -1;
            ret = 0;
        }
    sem_post(&statusMapSemaphore);

    return ret;
}
int InodeStatusMap::OpenForReading(Inode* inode)
{
    int ret;

    sem_wait(&statusMapSemaphore);
        if (statusMap.find(inode) != statusMap.end() && statusMap[inode] == -1)
        {
            errno = EACCES;
            ret = -1;
        }
        else
        {
            if (statusMap.find(inode) == statusMap.end())
            {
                statusMap[inode] = 0;
            }
            statusMap[inode]++;
            ret = 0;
        }
    sem_post(&statusMapSemaphore);

    return ret;
}
int InodeStatusMap::Close(Inode* inode)
{
    int ret;

    sem_wait(&statusMapSemaphore);
        if (statusMap.find(inode) == statusMap.end())
        {
            errno = EBADF;
            ret = -1;
        }
        else
        {
            if(statusMap[inode] == -1)
            {
                statusMap[inode] = 0;
            }
            else if(statusMap[inode] > 0)
            {
                statusMap[inode]--;
            }

            if(statusMap[inode] == 0)
            {
                statusMap.erase(inode);
            }
            ret = 0;
        }
    sem_post(&statusMapSemaphore);

    return ret;
}
InodeStatusMap::InodeStatusMap()
{
    sem_init(&statusMapSemaphore, 0 /* U SURE? */, 1);
}
InodeStatusMap::~InodeStatusMap()
{
    sem_destroy(&statusMapSemaphore);
}

FileDescriptor::FileDescriptor(Inode* inode, int mode, int number)
{
    this->inode = inode;
    this->mode = mode;
    this->number = number;
}

FileDescriptor* FileDescriptorProcessTable::CreateDescriptor(Inode* inode, int mode)
{
    FileDescriptor* fd = new FileDescriptor(inode, mode, fdNextNumber++);
    fdByFdNumber[fd->number] = fd;

    return fd;
}
int FileDescriptorProcessTable::DestroyDescriptor(int number)
{
    FileDescriptor* fd = fdByFdNumber[number];
    fdByFdNumber.erase(number);
    delete fd;

    return 0;
}
FileDescriptorProcessTable::~FileDescriptorProcessTable()
{
    std::map<int, FileDescriptor*>::iterator it;

    for ( it = fdByFdNumber.begin(); it != fdByFdNumber.end(); it++ )
    {
        delete it->second;
    }
}

FileDescriptor* FileDescriptorTable::CreateDescriptor(int pid, Inode* inode, int mode)
{
    sem_wait(&fdProcTableSemaphore);
        FileDescriptor* fd = fdProcTable[pid].CreateDescriptor(inode, mode);
    sem_post(&fdProcTableSemaphore);

    return fd;
}
int FileDescriptorTable::destroyDescriptor(int pid, int number)
{
    sem_wait(&fdProcTableSemaphore);
        int res = fdProcTable[pid].DestroyDescriptor(number);
    sem_post(&fdProcTableSemaphore);

    return res;
}
int FileDescriptorTable::destroyDescriptor(FileDescriptor* fd)
{
    int res;
    sem_wait(&fdProcTableSemaphore);
        int number = fd->number;
        int pid = -1;

        std::map<int, FileDescriptorProcessTable>::iterator itFdProcTable;

        // looks through every pid
        for (itFdProcTable = fdProcTable.begin(); itFdProcTable != fdProcTable.end(); itFdProcTable++)
        {
            std::map<int, FileDescriptor*>::iterator itFdByFdNumber;
            // looks through every fd in this pid
            for (itFdByFdNumber = itFdProcTable->second.fdByFdNumber.begin(); itFdByFdNumber != itFdProcTable->second.fdByFdNumber.end(); itFdByFdNumber++)
            {
                // okay, found, so get pid
                if(itFdByFdNumber->second == fd)
                {
                    pid = itFdProcTable->first;
                    break;
                }
            }
        }

        if (pid == -1) res = -1; // fd not found
        else res = fdProcTable[pid].DestroyDescriptor(number);
    sem_post(&fdProcTableSemaphore);


    return res;
}
FileDescriptor* FileDescriptorTable::getDescriptor(int pid, int number)
{
    FileDescriptor* fd;
    sem_wait(&fdProcTableSemaphore);
        if(fdProcTable.find(pid) == fdProcTable.end())
        {
            fd = nullptr;
        }
        else if(fdProcTable[pid].fdByFdNumber.find(number) == fdProcTable[pid].fdByFdNumber.end())
        {
            fd = nullptr;
        }
        else
        {
            fd = fdProcTable[pid].fdByFdNumber[number];
        }
    sem_post(&fdProcTableSemaphore);

    return fd;
}

FileDescriptorTable::FileDescriptorTable()
{
    sem_init(&fdProcTableSemaphore, 0 /* U SURE? */, 1);
}
FileDescriptorTable::~FileDescriptorTable()
{
    sem_destroy(&fdProcTableSemaphore);
}