#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include "management/DiskDescriptor.h"
#include "management/InodeList.h"
#include "management/UsageMap.h"
#include "diskfunctions/DiskOperations.h"

const int INODES_COUNT = 100;
const int BLOCK_SIZE  = 512;
const int FS_SIZE  = 512 * 100;

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
    printf("%d %d\n", id.inodeId, id.inodeAddress);
}

void printInodes()
{
    for (int i = 0; i < diskOps->ds->inodesCount; ++i)
        printInodeParams(i);
}

int main(int argc, const char** argv)
{
    if (INODES_COUNT < 1)
        return -1;
    else if (BLOCK_SIZE <= 0 || BLOCK_SIZE % 512 != 0)
        return -1;
    else if (FS_SIZE % BLOCK_SIZE != 0)
        return -1;


    diskOps = new DiskOperations(INODES_COUNT, BLOCK_SIZE, FS_SIZE);

    if (diskOps -> initShm() == -1)
    {
        printf("%d\n", errno);
        return 1;
    }
    diskOps -> initDiskStructures();
    diskOps -> initRoot();

    printUsageMap();
    printInodes();

    diskOps->mkdir("/XD1");
    diskOps->mkdir("/XD2");
    diskOps->mkdir("/XD3");

    printUsageMap();
    printInodes();

    delete diskOps;

    return 0;
}

