#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "management/DiskDescriptor.h"
#include "management/InodeList.h"
#include "management/UsageMap.h"
#include "diskfunctions/DiskOperations.h"

const char* VOLUME_NAME = "simplefs";
const unsigned int VOLUME_ID = 0;
const unsigned int INODES_COUNT = 100;
const unsigned int BLOCK_SIZE  = 512;
const unsigned int FS_SIZE  = 512 * 100;

DiskOperations* diskOps;

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

    diskOps = new DiskOperations(volumeName, volumeId, maxInodesCount, blockSize, fsSize);

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

    printUsageMap();
    printInodes();

    delete diskOps;
    return 0;
}

