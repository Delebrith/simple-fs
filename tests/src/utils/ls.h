#include <vector>
#include <string>

#include <fcntl.h>

#include "lib/src/simplefs.h"
#include "daemon/src/management/Directory.h"

std::vector<std::string> ls(const char* path)
{
	int fd = simplefs::simplefs_open(path, O_RDONLY);

	if (fd < 0)
        throw std::exception();

	char buf[sizeof(InodeDirectoryEntry)];
	if (simplefs::simplefs_read(fd, buf, sizeof(unsigned int)) != sizeof(unsigned int))
        throw std::exception();

    unsigned int entriesN = *reinterpret_cast<unsigned int*>(buf);

    std::vector<std::string> contents(entriesN);

    for (int i = 0; i < entriesN; ++i)
    {
        if (simplefs::simplefs_read(fd, buf, sizeof(InodeDirectoryEntry)) != sizeof(InodeDirectoryEntry))
        {
            simplefs::simplefs_close(fd);
            throw std::exception();
        }

        contents.push_back(std::string(reinterpret_cast<InodeDirectoryEntry*>(buf)->inodeName));
    }

    simplefs::simplefs_close(fd);
    return contents;
}
