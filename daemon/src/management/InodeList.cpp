#include "InodeList.h"
#include "DiskException.h"

InodeList::InodeList(unsigned int maxSize)
{
    this->maxSize = maxSize;
    this->inodesCount = 0;
    this->inodesArray = new InodeListEntry[maxSize];
}

void InodeList::addInodeEntry(InodeListEntry inodeListEntry)
{
    if (inodesCount == maxSize)
        throw DiskException("Too many inodes nodes");

    for (int i = 0; i < inodesCount; i++) {
        if (inodesArray[i].inodeId == inodeListEntry.inodeId)
            throw DiskException("Inode id duplicate");
    }
    inodesArray[inodesCount] = inodeListEntry;
    inodesCount++;
}

void InodeList::deleteInodeEntry(unsigned int inodeId)
{
    for (int i = 0; i < inodesCount; i++) {
        if (inodesArray[i].inodeId == inodeId){
            for (int j = i; j < inodesCount - 1; j++) {
                inodesArray[j] = inodesArray[j+1];
            }
            inodesCount--;
            return;
        }
    }
    throw DiskException("No such inode on the list");
}

InodeListEntry* InodeList::getById(unsigned int inodeId)
{
    for (int i = 0; i < inodesCount; i++) {
        if (inodesArray[i].inodeId == inodeId) {
            return &inodesArray[i];
        }
    }
    return nullptr;
}

InodeList::~InodeList()
{
    delete[] inodesArray;
}

