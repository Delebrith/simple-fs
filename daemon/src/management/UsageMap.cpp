#include <bits/exception.h>
#include "UsageMap.h"
#include "DiskException.h"

UsageMap::UsageMap(int size)
{
    this->size = size;
    this->blocks = new unsigned char[size];
    *this->blocks = {0};
}

void UsageMap::markBlocks(int from, int to, bool free)
{
    unsigned char value = free ? FREE : IN_USE;

    if (from < 0 || to >= size || from > to) {
        throw DiskException("Invalid bounds");
    }

    if (!free) {
        for (int i = from; i < to; i++) {
            if (blocks[i] == IN_USE)
                throw DiskException("Memory already in use");
        }
    }

    for (int i = from; i < to; i++) {
        blocks[i] = value;
    }
}

UsageMap::~UsageMap()
{
    delete[] blocks;
}