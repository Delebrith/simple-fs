#pragma once

struct DiskDescriptor
{
    char* volumeName;
    unsigned int volumeId;
    unsigned int blocksCount;
    unsigned int inodesCount;
};