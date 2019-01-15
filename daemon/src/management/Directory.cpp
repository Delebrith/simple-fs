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
	// this->getInodesArray() = new InodeDirectoryEntry[inodesCount];

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
	return (InodeDirectoryEntry*)(this + 1 /*sizeof(Directory)*/);
}

// reallocation BEFORE this function
void Directory::addEntry(InodeDirectoryEntry entry)
{
	InodeDirectoryEntry* inodesArrayAddr = getInodesArray();
	for (int i = 0; i < inodesCount; i++) {
		if (strcmp(inodesArrayAddr[i].inodeName, entry.inodeName) == 0)
			throw DiskException("Name duplication");
	}

   // InodeDirectoryEntry* old = this->getInodesArray();
  //  getInodesArray() = new InodeDirectoryEntry[inodesCount];

  //  for (int i = 0; i < inodesCount; i++) {
  //	  getInodesArray()[i] = old[i];
 //   }
	inodesArrayAddr[inodesCount++] = entry;

  //  delete[] old;
}

// reallocation AFTER this function
void Directory::deleteEntry(unsigned int inodeId)
{
	InodeDirectoryEntry* inodesArrayAddr = getInodesArray();
	for (int i = 0; i < inodesCount; i++)
	{
		if (inodesArrayAddr[i].inodeId == inodeId)
		{
			inodesCount--;

			for (int j = i; j < inodesCount; j++)
			{
				inodesArrayAddr[j] = inodesArrayAddr[j+1];
			}
		}
		break;
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
{
	//delete[] getInodesArray();
}
