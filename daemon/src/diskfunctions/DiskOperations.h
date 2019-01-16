#pragma once

#include "daemon/src/management/DiskDescriptor.h"
#include "daemon/src/management/UsageMap.h"
#include "daemon/src/management/InodeList.h"
#include "daemon/src/management/Inode.h"
#include "daemon/src/management/FileDescriptor.h"
#include "utils/src/IPCPackets.h"
#include <semaphore.h>


namespace simplefs
{

	struct DiskOperations
	{
		const unsigned int maxInodesCount;
		const unsigned int blockSize;
		const unsigned int fsSize;
		const unsigned int volumeId;
		const char* volumeName;

		DiskDescriptor* diskDescriptor;
		UsageMap* usageMap;
		InodeList* inodeList;
		Inode* inodes;
		FileDescriptorTable* fdTable;
		unsigned char* shmaddr;
		int shmid;
		key_t key;

		unsigned char* reallocate(Inode* inode, unsigned int newsize);

		unsigned char* getShmAddr(unsigned int blockIndex);

		Inode* getInodeById(unsigned int id);

		Inode* getFreeInodeAddr();

		int initShm();
		int initDiskStructures();
		int initRoot();

		Inode* createNewInodeEntry(unsigned int parentInodeId, Inode* inodeAddress, int inodeDataBlockAddress, int mode, int inodeFileType);

		DiskOperations(const char* volumeName, unsigned int volumeId, unsigned int maxInodesCount, unsigned int blockSize, unsigned int fsSize, FileDescriptorTable* fdTable);
		virtual ~DiskOperations();

		int fillInodeWithDirectoryData();
		Inode* dirNavigate(const char* path, int& error);
		Inode* getMember(Inode* parentDirInode, const char* name, int nameLen, int& error);
		Inode* getParent(const char* path, int pathLen, int &error);
		Inode* getInode(const char* path, int pathLen, int &error);
		Inode* createInode(const char* path, int mode, int inodeFileType);

		Packet* mkdir(const char* path, int permissions);
		Packet* open(const char* path, int flags, int pid); // Linux compatible flags
		Packet* openUsingFileDescriptorFlags(const char* path, int flags, int pid); // flags from FileDescriptor
		Packet* unlink(const char* path);
		Packet* create(const char* path, int mode, int pid);
		Packet* read(FileDescriptor* fd, int len);
		Packet* write(FileDescriptor* fd, int len);
		Packet* lseek(FileDescriptor* fd, int offset, int whence);
		Packet* chmod(const char* path, int mode);

		int linuxIntoFileDescriptorFlags(int flags);

		void printUsageMap();
		void printInodeParams(int i);
		void printInodes();
	private:
		sem_t inodeOpSemaphore;
	};
}
