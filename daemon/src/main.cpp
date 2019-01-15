#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <daemon/src/management/FileDescriptor.h>
#include "management/DiskDescriptor.h"
#include "management/InodeList.h"
#include "management/UsageMap.h"
#include "management/Directory.h"
#include "diskfunctions/DiskOperations.h"
#include "communication/ServerListener.h"
#include "management/FileDescriptor.h"
#include "logging/log.h"


const char* VOLUME_NAME = "simplefs";
const unsigned int VOLUME_ID = 0;
const unsigned int INODES_COUNT = 100;
const unsigned int BLOCK_SIZE  = 512;
const unsigned int FS_SIZE  = 512 * 100;

simplefs::DiskOperations* diskOps;
FileDescriptorTable* fdTable;

void printUsageMap()
{
	for (int x = 0; x < diskOps->um->size; ++x)
	{
		printf("%d ", diskOps->um->blocks[x]);
	}
	printf("\n");
}

void printInodeParams(int i)
{
	InodeListEntry id = diskOps->inodeList->inodesArray[i];
	printf("%d %p ", id.inodeId, id.inodeAddress);
	Inode* in = diskOps->getInodeById(id.inodeId);
	printf("%d %d\n", in->blockAddress, in->permissions);
}

void printInodes()
{
	for (int i = 0; i < diskOps->ds->inodesCount; ++i)
		printInodeParams(i);
}

void ls(Inode* inodeDirectory, simplefs::DiskOperations* diskOps)
{
	Directory* dir = (Directory*)diskOps->getShmAddr(inodeDirectory->blockAddress);
	InodeDirectoryEntry* directoryEntries = dir->getInodesArray();
	printf("LS, items: %d\n", dir->inodesCount);
	printf("name\tid\tperm\n");
	for(uint64_t x = 0; x < dir->inodesCount; x++)
	{
		printf("%s\t%d\t%d\n", directoryEntries->inodeName, directoryEntries->inodeId, diskOps->getInodeById(directoryEntries->inodeId)->permissions);
		directoryEntries++;
	}

}
void ls(char* dirName, simplefs::DiskOperations* diskOps)
{
	int error;
	Inode* inodeDirectory = diskOps->dirNavigate(dirName, error);

	Directory* dir = (Directory*)diskOps->getShmAddr(inodeDirectory->blockAddress);
	InodeDirectoryEntry* directoryEntries = dir->getInodesArray();
	printf("LS dirname: %s, items: %d\n", dirName, dir->inodesCount);
	printf("name\tid\tperm\n");
	for(uint64_t x = 0; x < dir->inodesCount; x++)
	{
		printf("%s\t%d\t%d\n", directoryEntries->inodeName, directoryEntries->inodeId, diskOps->getInodeById(directoryEntries->inodeId)->permissions);
		directoryEntries++;
	}

}

void runServer()
{
	simplefs::ServerListener listener;
	while (listener.isOk())
		listener.waitForConnection();
}

int main(int argc, const char** argv) // ./daemon.out vol_name vol_id fs_size block_size max_inodes_cnt
{
	const char* volumeName = VOLUME_NAME;
	unsigned int volumeId = VOLUME_ID;
	unsigned int fsSize = FS_SIZE;
	unsigned int blockSize = BLOCK_SIZE;
	unsigned int maxInodesCount = INODES_COUNT;

	if (argc > 1)
		volumeName = argv[1];
	if (argc > 2)
		volumeId = (unsigned int)strtol(argv[2], nullptr, 10);
	if (argc > 3)
		fsSize = (unsigned int)strtol(argv[3], nullptr, 10);
	if (argc > 4)
		blockSize = (unsigned int)strtol(argv[4], nullptr, 10);
	if (argc > 5)
		maxInodesCount = (unsigned int)strtol(argv[5], nullptr, 10);



	if (maxInodesCount < 1)
		return -1;
	else if (blockSize <= 0 || blockSize % 512 != 0)
		return -1;
	else if (fsSize % blockSize != 0)
		return -1;

	fdTable = new FileDescriptorTable();
	diskOps = new simplefs::DiskOperations(volumeName, volumeId, maxInodesCount, blockSize, fsSize, fdTable);


	if (diskOps->initShm() == -1)
	{
		delete diskOps;
		simplefs::log::logError("INIT", "Error creating memory: %d\n", errno);
		return 1;
	}
	else if (diskOps->initDiskStructures() == -1)
	{
		delete diskOps;
		simplefs::log::logError("INIT", "Error creating structures: %d\n", errno);
		return 1;
	}
	else if (diskOps->initRoot() == -1)
	{
		delete diskOps;
		simplefs::log::logError("INIT", "Error initializing root: %d\n", errno);
		return 1;
	}
	
	runServer();
	
	delete fdTable;
	delete diskOps;
	return 0;
}

