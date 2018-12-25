#pragma once

#include "InodeEntry.h"

struct InodeList
{
    unsigned int maxSize;
    unsigned int inodesCount;
    InodeListEntry* inodesArray;

    InodeList(unsigned int maxSize);
    void addInodeEntry(InodeListEntry inodeListEntry);
    void deleteInodeEntry(unsigned int inodeId);
    InodeListEntry* getById(unsigned int inodeId);
    ~InodeList();
};