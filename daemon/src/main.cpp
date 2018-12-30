#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include "management/DiskDescriptor.h"
#include "management/InodeList.h"
#include "management/UsageMap.h"

const int INODES_COUNT = 100;
const int BLOCK_SIZE = 512;
const int FS_SIZE = 512 * 100;


int inodetableaddr;

unsigned int ceil(unsigned int up, unsigned int down)
{
    unsigned int result = up / down;
    if (up % down != 0)
        ++result;
    result *= down;
    return result;
}

int main(int argc, const char** argv)
{
    if (INODES_COUNT < 1)
        return -1;
    else if (BLOCK_SIZE <= 0 || BLOCK_SIZE % 512 != 0)
        return -1;
    else if (FS_SIZE % BLOCK_SIZE != 0)
        return -1;
    // TO-DO: create shm file
    key_t key = ftok("/dev/shm/miau", IPC_PRIVATE);
    if (key == -1)
    {
        printf("%d\n", errno);
        return -1;
    }
    int shmid = shmget(key, FS_SIZE, 0777 | IPC_CREAT);
    if (shmid == -1)
    {
        printf("%d\n", errno);
        return -1;
    }
    char* shmaddr = (char*)shmat(shmid, (void*)0,0);

    DiskDescriptor* ds = (DiskDescriptor*)shmaddr;
    ds->blocksCount = FS_SIZE / BLOCK_SIZE;
    ds->inodesCount = INODES_COUNT;
    ds->volumeId = 0;
    ds->volumeName = "XD ヽ༼ຈل͜ຈ༽ﾉ";

    unsigned char* bitmap = (unsigned char*)(shmaddr+BLOCK_SIZE);
   // bitmap[10] = (unsigned char)4;
   // for (int i = 0; i < 12; ++i)
   //     printf("%d", bitmap[i]);
    UsageMap* um = new UsageMap(ds->blocksCount, bitmap);

    inodetableaddr = BLOCK_SIZE + ceil(ds->blocksCount, BLOCK_SIZE);

    // TO-DO: Mark used bloki iksde

    shmdt(shmaddr);

    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}

