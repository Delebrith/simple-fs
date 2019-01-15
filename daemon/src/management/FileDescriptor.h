#pragma once

//
// Created by nkp123 on 12.01.19.
//

#include <map>
#include <vector>
#include <errno.h>
#include <semaphore.h>

#include "Inode.h"



struct InodeStatusMap
{
	// value:
	// -1 = opened for writing (and eventually reading by the same process -> not implemented as for now)
	//  0 = not opened (but in this case that Inode* will be removed from map. Only OpenStatus function returns 0.)
	// >0 = opened for reading by x readers
	// key:
	// pointer to inode
	std::map<Inode*, int> statusMap;

	// returns above stated values
	int InodeStatus(Inode* inode);
	int OpenForReadWrite(Inode* inode);
	int OpenForWriting(Inode* inode);
	int OpenForReading(Inode* inode);
	int Close(Inode* inode);

	InodeStatusMap();
	~InodeStatusMap();

private:
	sem_t statusMapSemaphore;
};

struct FileDescriptor
{
	const static unsigned char M_READ = 1;
	const static unsigned char M_WRITE = 2;
	const static unsigned char M_CREATE = 4;
	const static unsigned char M_CREATE_PERM_R = 8;
	const static unsigned char M_CREATE_PERM_W = 16;
	const static unsigned char M_CREATE_PERM_X = 32;

	Inode* inode;
	int mode;
	int number;

	unsigned long long position = 0;

	FileDescriptor(Inode* inode, int mode, int number);
};

struct FileDescriptorProcessTable
{
	std::map<int, FileDescriptor*> fdByFdNumber;

	// as we are only accessing this from FileDescriptorTable, which is synchronized, we don't need to synchronize this
	FileDescriptor* CreateDescriptor(Inode* inode, int mode);
	int DestroyDescriptor(int number);

	~FileDescriptorProcessTable();

private:
	int fdNextNumber = 0;
};

struct FileDescriptorTable
{
	InodeStatusMap inodeStatusMap;



	FileDescriptor* CreateDescriptor(int pid, Inode* inode, int mode);
	int destroyDescriptor(int pid, int number);
	int destroyDescriptor(FileDescriptor* fd);

	FileDescriptor* getDescriptor(int pid, int number);

	FileDescriptorTable();
	~FileDescriptorTable();

private:
	sem_t fdProcTableSemaphore;
	std::map<int, FileDescriptorProcessTable> fdProcTable;
};
