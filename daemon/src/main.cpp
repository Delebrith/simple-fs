#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <daemon/src/management/FileDescriptor.h>
#include "management/DiskDescriptor.h"
#include "management/InodeList.h"
#include "management/UsageMap.h"
#include "management/Directory.h"
#include "diskfunctions/DiskOperations.h"
#include "communication/ServerListener.h"
#include "management/FileDescriptor.h"

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
    printf("%d %d ", id.inodeId, id.inodeAddress);
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
    for(uint64_t x = 0; x < dir->inodesCount; x++)
    {
        printf("%s\t%d\n", directoryEntries->inodeName, directoryEntries->inodeId);
        directoryEntries++;
    }

}
void ls(char* dirName, simplefs::DiskOperations* diskOps)
{
    Inode* inodeDirectory = diskOps->dirNavigate(dirName);

    Directory* dir = (Directory*)diskOps->getShmAddr(inodeDirectory->blockAddress);
    InodeDirectoryEntry* directoryEntries = dir->getInodesArray();
    printf("LS dirname: %s, items: %d\n", dirName, dir->inodesCount);
    for(uint64_t x = 0; x < dir->inodesCount; x++)
    {
        printf("%s\t%d\n", directoryEntries->inodeName, directoryEntries->inodeId);
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
        printf("%d\n", errno);
        return 1;
    }

    diskOps->initDiskStructures();
    diskOps->initRoot();
    printUsageMap();
    printInodes();

    diskOps->mkdir("/XD1", 6);
    diskOps->mkdir("/XD2", 6);
    diskOps->mkdir("/XD3", 6);
    diskOps->mkdir("/XD1/XD1", 6);
    diskOps->mkdir("/XD2/XD2", 6);
    diskOps->mkdir("/XD3/XD3", 6);
    diskOps->mkdir("/XD1/XD2", 6);
    diskOps->mkdir("/XD1/XD3", 6);
    diskOps->mkdir("/XD1/XD4", 6);
    diskOps->mkdir("/XD1/XD5", 6);
    diskOps->mkdir("/XD1/XD6", 6);
    diskOps->mkdir("/XD1/XD10", 6);
    diskOps->mkdir("/XDD", 6);
    diskOps->create("/1", 6, 1);
    diskOps->create("/2", 6, 1);
    diskOps->create("/2", 6, 1);
    diskOps->create("/3", 6, 1);
    diskOps->open("/4", FileDescriptor::M_CREATE, 1);
    diskOps->create("/5", 6, 1);
    diskOps->create("/6", 6, 1);
    diskOps->create("/7", 6, 1);
    diskOps->create("/8", 6, 1);

    fdTable->getDescriptor(1,4);
    printf("\nopened node number at fd=4 and pid=1 (should be 4): %d\n", fdTable->getDescriptor(1,4)->number);

    Inode* resinode = 0;
    printf("\ndir navigation test before: %ld\n", (uint64_t)resinode);
    resinode = diskOps->dirNavigate("/XD1");
    printf("dir navigation testafter (should be nonzero): %ld\n\n", (uint64_t)resinode);
    //resinode = diskOps->getInodeById(0);
    ls("/XD1",diskOps);
    ls("/",diskOps);


    printUsageMap();
    printInodes();

    //runServer(); Uncomment to actually run program


    // FILE DESCRIPTOR "TESTS"
    fdTable->CreateDescriptor(10, diskOps->getInodeById(0), 1);
    fdTable->CreateDescriptor(10, diskOps->getInodeById(1), 1);
    FileDescriptor* miau = fdTable->CreateDescriptor(11, diskOps->getInodeById(1), 1);
    printf("\n\nTEST: %ld\n", (uint64_t)diskOps->getInodeById(2));
    fdTable->CreateDescriptor(12, diskOps->getInodeById(2), 1);

    printf("\n\nFileDescriptorTable one of the inodes from fd list:\n%d\n", fdTable->getDescriptor(12, 0)->inode->id);

    fdTable->destroyDescriptor(miau);
    fdTable->destroyDescriptor(12, 0);

    printf("if nullptr then destroyed:\n%d\n", (uint64_t)(fdTable->getDescriptor(12, 0)));


    printf("\nRoot dir inode id: %d\n", (diskOps->getInodeById(0)->id));
    // INODESTATUSMAP "TESTS"
    fdTable->inodeStatusMap.OpenForReading(diskOps->getInodeById(0));
    fdTable->inodeStatusMap.OpenForReading(diskOps->getInodeById(0));
    fdTable->inodeStatusMap.OpenForWriting(diskOps->getInodeById(1));
    printf("\n\nInodeStatusMap:\n%d\n%d\n",
            fdTable->inodeStatusMap.InodeStatus(diskOps->getInodeById(0)),
            fdTable->inodeStatusMap.InodeStatus(diskOps->getInodeById(1))
    );

    fdTable->inodeStatusMap.Close(diskOps->getInodeById(0));
    fdTable->inodeStatusMap.Close(diskOps->getInodeById(1));
    printf("\n%d\n%d\n",
           fdTable->inodeStatusMap.InodeStatus(diskOps->getInodeById(0)),
           fdTable->inodeStatusMap.InodeStatus(diskOps->getInodeById(1))
    );

    fdTable->inodeStatusMap.Close(diskOps->getInodeById(0));
    printf("\n%d\n%d\n",
           fdTable->inodeStatusMap.InodeStatus(diskOps->getInodeById(0)),
           fdTable->inodeStatusMap.InodeStatus(diskOps->getInodeById(1))
    );

    delete diskOps;
    return 0;
}

