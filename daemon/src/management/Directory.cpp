#include <string>
#include <cstring>
#include "Directory.h"
#include "DiskException.h"

Directory::Directory(unsigned int parentInodeId, unsigned int inodeId)
{
    this->inodesCount = 2;
    this->inodesArray = new InodeDirectoryEntry[inodesCount];

    // parent directory
    inodesArray[0].inodeId = parentInodeId;
    inodesArray[0].inodeName = const_cast<char *>("..");

    // newly created directory
    inodesArray[1].inodeId = inodeId;
    inodesArray[1].inodeName = const_cast<char *>(".");
}

void Directory::addEntry(InodeDirectoryEntry entry)
{
    for (int i = 0; i < inodesCount; i++) {
        if (strcmp(inodesArray[i].inodeName, entry.inodeName) == 0)
            throw DiskException("Name duplication");
    }

    InodeDirectoryEntry* old = this->inodesArray;
    inodesArray = new InodeDirectoryEntry[inodesCount];

    for (int i = 0; i < inodesCount; i++) {
        inodesArray[i] = old[i];
    }
    inodesArray[inodesCount] = entry;
    inodesCount++;

    delete[] old;
}

void Directory::deleteEntry(unsigned int inodeId)
{
    for (int i = 0; i < inodesCount; i++) {
        if (inodesArray[i].inodeId == inodeId){
            InodeDirectoryEntry* old = inodesArray;
            inodesArray = new InodeDirectoryEntry[inodesCount--];

            for (int j = i; j < inodesCount; j++) {
                inodesArray[j] = old[j+1];
            }

            delete[] old;
        }
        break;
    }
    throw DiskException("No such inode in the directory");
}


InodeDirectoryEntry* Directory::getByName(char *inodeName)
{
    for (int i = 0; i < inodesCount; i++) {
        if (strcmp(inodesArray[i].inodeName, inodeName) == 0)
            return &inodesArray[i];
    }
    return nullptr;
}

const char* Directory::listDirectory()
{
    std::string content;
    for (int i = 0; i < inodesCount; i++) {
        content.append(inodesArray[i].inodeName);
        content.append("\n");
    }
    const char* result = content.c_str();
    return result;
}

Directory::~Directory()
{
    delete[] inodesArray;
}