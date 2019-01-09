#pragma once

struct DiskDescriptor
{
    char volumeName[64];
    unsigned int volumeId;
    unsigned int blocksCount;
    unsigned int inodesCount;
    unsigned int maxInodesCount;
    unsigned int freeInodeId;
};