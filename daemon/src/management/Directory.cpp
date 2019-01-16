#include <string>
#include <cstring>
#include "Directory.h"
#include "DiskException.h"

Directory::Directory(unsigned int parentInodeId, unsigned int inodeId)
{
	init(parentInodeId, inodeId);
}

void Directory::init(unsigned int parentInodeId, unsigned int inodeId)
{
	this->inodesCount = 2;

	InodeDirectoryEntry* inodesArray = getInodesArray();
	// parent directory
	inodesArray[0].inodeId = parentInodeId;
	strcpy(inodesArray[0].inodeName, "..");

	// newly created directory
	inodesArray[1].inodeId = inodeId;
	strcpy(inodesArray[1].inodeName, ".");
}

unsigned int Directory::getSize()
{
	return sizeof(Directory) + inodesCount * sizeof(InodeDirectoryEntry);
}

InodeDirectoryEntry* Directory::getInodesArray()
{
	return (InodeDirectoryEntry*)(this + 1);
}

// reallocation BEFORE this function
void Directory::addEntry(InodeDirectoryEntry entry)
{
	InodeDirectoryEntry* inodesArrayAddr = getInodesArray();
	for (int i = 0; i < inodesCount; i++)
	{
		if (strcmp(inodesArrayAddr[i].inodeName, entry.inodeName) == 0)
			throw DiskException("Name duplication");
	}
	inodesArrayAddr[inodesCount++] = entry;
}

#include <iostream>
// reallocation AFTER this function
void Directory::deleteEntry(unsigned int inodeId)
{
	std::cout << "DELETING INODE ENTRY " << inodeId << std::endl;
	InodeDirectoryEntry* inodesArrayAddr = getInodesArray();
	for (int i = 0; i < inodesCount; i++)
	{
	std::cout << "CHECKING INODE ENTRY " <<inodesArrayAddr[i].inodeName << " OF " << inodesCount << " ID: " << inodesArrayAddr[i].inodeId << std::endl;
		if (inodesArrayAddr[i].inodeId == inodeId)
		{
			inodesCount--;

			for (int j = i; j < inodesCount; j++)
			{
				inodesArrayAddr[j] = inodesArrayAddr[j+1];
			}

			return;
		}
	}
	throw DiskException("No such inode in the directory");
}


InodeDirectoryEntry* Directory::getByName(char *inodeName)
{
	for (int i = 0; i < inodesCount; i++) {
		if (strcmp(getInodesArray()[i].inodeName, inodeName) == 0)
			return &getInodesArray()[i];
	}
	return nullptr;
}

const char* Directory::listDirectory()
{
	std::string content;
	for (int i = 0; i < inodesCount; i++) {
		content.append(getInodesArray()[i].inodeName);
		content.append("\n");
	}
	const char* result = content.c_str();
	return result;
}

Directory::~Directory()
{}
