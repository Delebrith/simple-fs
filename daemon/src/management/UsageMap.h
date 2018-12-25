#pragma once

struct UsageMap
{
    const static unsigned char FREE = '\000';
    const static unsigned char IN_USE = '1';

    int size;
    unsigned char* blocks;

    UsageMap(int size);

    // free = true if blocks will be freed, false if blocks will be allocated
    void markBlocks(int from, int to, bool free);

    ~UsageMap();
};