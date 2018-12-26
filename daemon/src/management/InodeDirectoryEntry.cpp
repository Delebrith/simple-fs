#include <cstring>
#include "InodeEntry.h"

InodeDirectoryEntry::InodeDirectoryEntry(unsigned int inodeId, const char *inodeName)
{
    this->inodeId = inodeId;
    this->inodeName = new char[strlen(inodeName) + 1];
    strcpy(this->inodeName, inodeName);
}

InodeDirectoryEntry::InodeDirectoryEntry() {}

InodeDirectoryEntry::~InodeDirectoryEntry()
{
    delete[] inodeName;
}