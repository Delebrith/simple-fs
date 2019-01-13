#pragma once

#include <semaphore.h>

struct UsageMap
{
    const static unsigned char FREE = '\000';
    const static unsigned char IN_USE = '1';

    int size;
    unsigned char* blocks;

   // UsageMap(int size);
    UsageMap(int size, unsigned char* addr);

    // free = true if blocks will be freed, false if blocks will be allocated
    void markBlocks(int from, int to, bool free, bool lock);

    int getFreeBlocks(unsigned int requiredBlocks, bool lock);

    void lock();
    void unlock();

    ~UsageMap();

private:
    sem_t usageMapSemaphore;
};
