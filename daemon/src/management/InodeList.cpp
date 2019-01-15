#include "InodeList.h"
#include "DiskException.h"

InodeList::InodeList(DiskDescriptor* dd, InodeListEntry* addr)
{
	this->diskDescriptor = dd;
	this->inodesArray = addr;
	sem_init(&inodeListSemaphore, 0, 1);
}

void InodeList::addInodeEntry(InodeListEntry inodeListEntry)
{
	sem_wait(&inodeListSemaphore);
	if (diskDescriptor->inodesCount == diskDescriptor->maxInodesCount)
	{
		sem_post(&inodeListSemaphore);
		throw DiskException("Too many inodes nodes");
	}
	for (int i = 0; i < diskDescriptor->inodesCount; i++)
	{
		if (inodesArray[i].inodeId == inodeListEntry.inodeId)
		{
			sem_post(&inodeListSemaphore);
			throw DiskException("Inode id duplicate");
		}
	}
	inodesArray[diskDescriptor->inodesCount] = inodeListEntry;
	diskDescriptor->inodesCount++;
	sem_post(&inodeListSemaphore);
}

void InodeList::deleteInodeEntry(unsigned int inodeId)
{
	sem_wait(&inodeListSemaphore);
	for (int i = 0; i < diskDescriptor->inodesCount; i++)
	{
		if (inodesArray[i].inodeId == inodeId)
		{
			for (int j = i; j < diskDescriptor->inodesCount - 1; j++)
			{
				inodesArray[j] = inodesArray[j+1];
			}
			diskDescriptor->inodesCount--;
			sem_post(&inodeListSemaphore);
			return;
		}
	}
	sem_post(&inodeListSemaphore);
	throw DiskException("No such inode on the list");
}

InodeListEntry* InodeList::getById(unsigned int inodeId)
{
	sem_wait(&inodeListSemaphore);
	for (int i = 0; i < diskDescriptor->inodesCount; i++)
	{
		if (inodesArray[i].inodeId == inodeId)
		{
			sem_post(&inodeListSemaphore);
			return &inodesArray[i];
		}
	}
	sem_post(&inodeListSemaphore);
	return nullptr;
}

InodeList::~InodeList()
{
	// delete[] inodesArray;
	sem_destroy(&inodeListSemaphore);
}

