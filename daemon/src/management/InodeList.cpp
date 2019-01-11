#include "InodeList.h"
#include "DiskException.h"

InodeList::InodeList(DiskDescriptor* dd, InodeListEntry* addr)
{
    this->diskDescriptor = dd;
    this->inodesArray = addr;
}

void InodeList::addInodeEntry(InodeListEntry inodeListEntry)
{
    if (diskDescriptor->inodesCount == diskDescriptor->maxInodesCount)
        throw DiskException("Too many inodes nodes");

    for (int i = 0; i < diskDescriptor->inodesCount; i++) {
        if (inodesArray[i].inodeId == inodeListEntry.inodeId)
            throw DiskException("Inode id duplicate");
    }
    inodesArray[diskDescriptor->inodesCount] = inodeListEntry;
    diskDescriptor->inodesCount++;
}

void InodeList::deleteInodeEntry(unsigned int inodeId)
{
    for (int i = 0; i < diskDescriptor->inodesCount; i++) {
        if (inodesArray[i].inodeId == inodeId){
            for (int j = i; j < diskDescriptor->inodesCount - 1; j++) {
                inodesArray[j] = inodesArray[j+1];
            }
            diskDescriptor->inodesCount--;
            return;
        }
    }
    throw DiskException("No such inode on the list");
}

InodeListEntry* InodeList::getById(unsigned int inodeId)
{
    for (int i = 0; i < diskDescriptor->inodesCount; i++) {
        if (inodesArray[i].inodeId == inodeId) {
            return &inodesArray[i];
        }
    }
    return nullptr;
}

InodeList::~InodeList()
{
    // delete[] inodesArray;
}

