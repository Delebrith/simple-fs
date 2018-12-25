#pragma once

#include "InodeEntry.h"

struct Directory
{
    unsigned int inodesCount;
    InodeDirectoryEntry *inodesArray;

    Directory(unsigned int parentInodeId, unsigned int inodeId);
    void addEntry(InodeDirectoryEntry entry);
    void deleteEntry(unsigned int inodeId);
    InodeDirectoryEntry* getByName(char* inodeName);
    const char* listDirectory();
    ~Directory();
};