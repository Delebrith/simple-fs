#pragma once

#include "InodeEntry.h"

struct Directory
{
	unsigned int inodesCount;

	unsigned int getSize();


	InodeDirectoryEntry* getInodesArray();

	Directory(unsigned int parentInodeId, unsigned int inodeId);
	void init(unsigned int parentInodeId, unsigned int inodeId);
	// If you want to modify name of the entry delete one entry
	// and replace it with new one with method below to avoid conflicting names
	void addEntry(InodeDirectoryEntry entry);
	void deleteEntry(unsigned int inodeId);

	const char* listDirectory();

	InodeDirectoryEntry* getByName(char* inodeName);

	~Directory();
};
