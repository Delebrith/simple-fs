#include <exception>
#include "UsageMap.h"
#include "DiskException.h"

/*UsageMap::UsageMap(int size)
{
    this->size = size;
    this->blocks = new unsigned char[size];
    *this->blocks = {0};
}*/

UsageMap::UsageMap(int size, unsigned char *addr)
{
    this->size = size;
    this->blocks = addr;
    for (int x = 0; x < size; ++x)
        *(this->blocks + x) = 0;
    sem_init(&usageMapSemaphore, 0, 1);

}

void UsageMap::markBlocks(int from, int to, bool free)
{
    unsigned char value = free ? FREE : IN_USE;

    if (from < 0 || to >= size || from > to) {
        throw DiskException("Invalid bounds");
    }
    sem_wait(&usageMapSemaphore);
    if (!free) {
        for (int i = from; i < to; i++) {
            if (blocks[i] == IN_USE)
            {
                sem_post(&usageMapSemaphore);
                throw DiskException("Memory already in use");
            }
        }
    }

    for (int i = from; i < to; i++) {
        blocks[i] = value;
    }
    sem_post(&usageMapSemaphore);
}

int UsageMap::getFreeBlocks(unsigned int requiredBlocks)
{
    unsigned int freeBlocks = 0;
    unsigned int firstBlock = 0;
    sem_wait(&usageMapSemaphore);
    for (int i = 0; i < size; ++i)
    {
        if (blocks[i] == FREE)
        {
            if (freeBlocks == 0)
                firstBlock = i;
            ++freeBlocks;
        }
        else
            freeBlocks = 0;
        if (freeBlocks == requiredBlocks)
        {
            sem_post(&usageMapSemaphore);
            return firstBlock;
        }
    }
    sem_post(&usageMapSemaphore);
    return -1;
}

UsageMap::~UsageMap()
{
    // delete[] blocks;
    sem_destroy(&usageMapSemaphore);
}
