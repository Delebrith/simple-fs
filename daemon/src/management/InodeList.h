#pragma once

#include "InodeEntry.h"
#include "DiskDescriptor.h"
#include <semaphore.h>

struct InodeList
{
    DiskDescriptor* diskDescriptor;
    InodeListEntry* inodesArray;

    InodeList(DiskDescriptor* dd, InodeListEntry* addr);
    void addInodeEntry(InodeListEntry inodeListEntry);
    void deleteInodeEntry(unsigned int inodeId);
    InodeListEntry* getById(unsigned int inodeId);
    ~InodeList();

private:
    sem_t inodeListSemaphore;
};