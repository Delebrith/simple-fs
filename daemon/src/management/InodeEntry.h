#pragma once

struct InodeEntry
{
    unsigned int inodeId;
};

struct InodeDirectoryEntry: InodeEntry
{
    char* inodeName;
};

struct InodeListEntry: InodeEntry
{
    unsigned int inodeAddress;
};