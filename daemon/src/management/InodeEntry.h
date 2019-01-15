#pragma once
#include "Inode.h"

struct InodeEntry
{
	unsigned int inodeId;
};

struct InodeDirectoryEntry: InodeEntry
{
	// please, do not modify inodeName after adding to directory to avoid conflicting names
	// char* inodeName = nullptr;
	char inodeName[256];
	InodeDirectoryEntry();
	InodeDirectoryEntry(unsigned int inodeId, const char* inodeName);
	void init(unsigned int inodeId, const char* inodeName);
	~InodeDirectoryEntry();
};

struct InodeListEntry: InodeEntry
{
	Inode* inodeAddress;
};
