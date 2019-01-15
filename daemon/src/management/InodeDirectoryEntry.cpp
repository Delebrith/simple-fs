#include <cstring>
#include "InodeEntry.h"

InodeDirectoryEntry::InodeDirectoryEntry(unsigned int inodeId, const char *inodeName)
{
	init(inodeId, inodeName);
}

void InodeDirectoryEntry::init(unsigned int inodeId, const char *inodeName)
{
	this->inodeId = inodeId;
	strcpy(this->inodeName, inodeName);
}

InodeDirectoryEntry::InodeDirectoryEntry() {}

InodeDirectoryEntry::~InodeDirectoryEntry()
{}
