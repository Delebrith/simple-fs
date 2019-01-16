#include <sys/ipc.h>
#include "DiskOperations.h"
#include "../management/Directory.h"
#include <errno.h>
#include <cstdio>
#include <sys/shm.h>
#include <cstring>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>	// std::max


#include <iostream>

using namespace simplefs;

inline int ceil(int value, int divisor)
{
	return value / divisor + (value % divisor != 0);
}

unsigned char* DiskOperations::reallocate(Inode* inode, unsigned int newSize)
{
std::cout << "REALLOCATE FROM " << inode->nodeSize << " B TO " << newSize << " B\n";
	unsigned int oldBlocks = ceil(inode->nodeSize, blockSize);
	unsigned int newBlocks = ceil(newSize, blockSize);
	unsigned char* toRet = nullptr;
	if (oldBlocks == newBlocks)
	{
		inode->nodeSize = newSize;
		return getShmAddr(inode->blockAddress);
	}
	else if (oldBlocks > newBlocks)
	{
		inode->nodeSize = newSize;
		usageMap->markBlocks(inode->blockAddress + newBlocks, inode->blockAddress + oldBlocks, true, false);
		return getShmAddr(inode->blockAddress);
	}
	else
	{
		usageMap->lock();

		bool canExtend = true;
		for (unsigned int x = inode->blockAddress + oldBlocks; x < inode->blockAddress + newBlocks; ++x)
		{
			if (usageMap->blocks[x] == UsageMap::IN_USE)
			{
				canExtend = false;
				break;
			}
		}

		if (canExtend)
		{
			usageMap->markBlocks(inode->blockAddress + oldBlocks, inode->blockAddress + newBlocks, false, false);
			toRet = getShmAddr(inode->blockAddress);
		}
		else
		{
			int freeBlocksStart = usageMap->getFreeBlocks(newBlocks, false);
			if (freeBlocksStart != -1)
			{
				toRet = getShmAddr((unsigned int)freeBlocksStart);
				usageMap->markBlocks(freeBlocksStart, freeBlocksStart + newBlocks, false, false);
				memcpy(toRet, getShmAddr(inode->blockAddress), inode->nodeSize);
				usageMap->markBlocks(inode->blockAddress, inode->blockAddress + oldBlocks, true, false);
				inode->blockAddress = (unsigned int)freeBlocksStart;
			}
		}

		inode->nodeSize = newSize;

		usageMap->unlock();
		return toRet;
	}
}

DiskOperations::DiskOperations(const char* volumeName, const unsigned int volumeId, const unsigned int maxInodesCount,
		const unsigned int blockSize, const unsigned int fsSize, FileDescriptorTable* fdTable):
		volumeName(volumeName), volumeId(volumeId), maxInodesCount(maxInodesCount), blockSize(blockSize), fsSize(fsSize), fdTable(fdTable)
{
	sem_init(&inodeOpSemaphore, 0, 1);
}

unsigned char* DiskOperations::getShmAddr(unsigned int blockIndex)
{
	return shmaddr + blockIndex * blockSize;
}

Inode* DiskOperations::getInodeById(unsigned int id)
{
	for (int i = 0; i < diskDescriptor->inodesCount; ++i)
	{
		if (inodeList->inodesArray[i].inodeId == id)
			return inodeList->inodesArray[i].inodeAddress;
	}
	return nullptr;
}

Inode *DiskOperations::getFreeInodeAddr() {
	return inodes + diskDescriptor->inodesCount;
}

int DiskOperations::initShm()
{
	unsigned int devShmStrLen = 10;
	char* shmFilePath = new char[devShmStrLen + strlen(volumeName)];
	strcat(shmFilePath, "/dev/shm/");
	strcat(shmFilePath, volumeName);
	int createdShmFd = creat(shmFilePath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (createdShmFd == -1)
	{
		delete[] shmFilePath;
		return -1;
	}
	close(createdShmFd);
	key = ftok(shmFilePath, IPC_PRIVATE);
	delete[] shmFilePath;
	if (key == -1)
		return -1;

	shmid = shmget(key, fsSize, 0777 | IPC_CREAT);
	if (shmid == -1)
		return -1;

	shmaddr = (unsigned char*)shmat(shmid, nullptr, 0);

	return 0;
}

int DiskOperations::initDiskStructures()
{
	diskDescriptor = (DiskDescriptor*)shmaddr;
	diskDescriptor->blocksCount = fsSize / blockSize;
	diskDescriptor->maxInodesCount = maxInodesCount;
	diskDescriptor->inodesCount = 0;
	diskDescriptor->volumeId = volumeId;
	diskDescriptor->freeInodeId = 0;
	strcpy(diskDescriptor->volumeName, volumeName);

	int numBlocks = ceil(sizeof(DiskDescriptor), blockSize)
					+ ceil(diskDescriptor->blocksCount, blockSize) + ceil(sizeof(InodeListEntry) * diskDescriptor->maxInodesCount, blockSize)
					+ ceil(sizeof(Inode) * diskDescriptor->maxInodesCount, blockSize);

	if (numBlocks > diskDescriptor->blocksCount)
		return -1;

	unsigned char* bitmap = shmaddr + blockSize;
	usageMap = new UsageMap(diskDescriptor->blocksCount, bitmap);

	InodeListEntry* inodeTableAddr = (InodeListEntry*) (shmaddr + ceil(sizeof(DiskDescriptor), blockSize) * blockSize
			+ ceil(diskDescriptor->blocksCount, blockSize) * blockSize);

	inodeList = new InodeList(diskDescriptor, inodeTableAddr);

	inodes = (Inode*)(shmaddr + ceil(sizeof(DiskDescriptor), blockSize) * blockSize
								 + ceil(diskDescriptor->blocksCount, blockSize) * blockSize
								 + ceil(sizeof(InodeListEntry) * diskDescriptor->maxInodesCount, blockSize) * blockSize);
	usageMap->markBlocks(0, numBlocks, false, true);

	return 0;
}

int DiskOperations::initRoot()
{
	unsigned int inodeDataBlocks = ceil(2 * sizeof(InodeDirectoryEntry) + sizeof(Directory), blockSize);
	int inodeDataBlockAddress = usageMap->getFreeBlocks(inodeDataBlocks, false);
	if (inodeDataBlockAddress == -1)
		return -1;
	usageMap->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress + inodeDataBlocks, false, false);

	createNewInodeEntry(0, getFreeInodeAddr(), inodeDataBlockAddress, Inode::PERM_R | Inode::PERM_W, Inode::IT_DIRECTORY);

	return 0;
}

Inode* DiskOperations::createNewInodeEntry(unsigned int parentInodeId, Inode* inodeAddress, int inodeDataBlockAddress, int mode, int inodeFileType)
{
	Inode* inode = inodeAddress;

	inode->id = diskDescriptor->freeInodeId++;
	inode->fileType = inodeFileType;
	inode->permissions = mode & (Inode::PERM_W | Inode::PERM_R | Inode::PERM_X);
	inode->modificationDate = time(0);
	inode->accessDate = time(0);
	inode->creationDate = time(0);
	inode->deletionDate = 0;
	inode->blockAddress = (unsigned int)inodeDataBlockAddress;

std::cout << "CREATING INODE, parent: " << parentInodeId << " id: " << inode->id <<std::endl;

	inode->nodeSize = 0;

	if(inodeFileType == Inode::IT_DIRECTORY)
	{
		Directory* directory = (Directory*)(getShmAddr((unsigned int)inodeDataBlockAddress));
		directory->init(parentInodeId, inode->id);
		inode->nodeSize = directory->getSize();
	}

	InodeListEntry entry{};
	entry.inodeAddress = inodeAddress;
	entry.inodeId = inode->id;

	inodeList->addInodeEntry(entry);

	return inode;
}

DiskOperations::~DiskOperations()
{
	delete usageMap;
	delete inodeList;
	sem_destroy(&inodeOpSemaphore);

	shmdt(shmaddr);

	shmctl(shmid, IPC_RMID, nullptr);
}

void DiskOperations::printUsageMap()
{
	for (int x = 0; x < usageMap->size; ++x)
	{
		printf("%d ", usageMap->blocks[x]);
	}
	printf("\n");
}

void DiskOperations::printInodeParams(int i)
{
	InodeListEntry id = inodeList->inodesArray[i];
	printf("%d %p ", id.inodeId, id.inodeAddress);
	Inode* in = getInodeById(id.inodeId);
	printf("%d %d\n", in->blockAddress, in->permissions);
}

void DiskOperations::printInodes()
{
	for (int i = 0; i < diskDescriptor->inodesCount; ++i)
		printInodeParams(i);
}

int getLastMemberLen(const char* path, int pathLen)
{
	if (pathLen == 1)
		return 1;

	int len = 1;

	for (const char* ch = path + pathLen - 2; ch != path; ++len, --ch)
	{
		if (*ch == '/')
			return len;
	}

	return len;
}

Inode* DiskOperations::getMember(Inode* parentDirInode, const char* name, int nameLen, int& error)
{
std::cout << "GET MEMBER: " << name << " len: " << nameLen << std::endl;
	if (!(parentDirInode->permissions & Inode::PERM_R))
	{
		error = EPERM;
	}

	Directory* dir = (Directory*)getShmAddr(parentDirInode->blockAddress);

	int len = nameLen;
	if (name[nameLen - 1] == '/')
		--len;

std::cout << name << std::endl;

	for (int i = 0; i < dir->inodesCount; ++i)
	{
std::cout << "IAN " << dir->getInodesArray()[i].inodeName << " id: " << dir->getInodesArray()[i].inodeId << std::endl;
		if (strncmp(dir->getInodesArray()[i].inodeName, name, len) == 0 && dir->getInodesArray()[i].inodeName[len] == 0)
			return getInodeById(dir->getInodesArray()[i].inodeId);
	}
	error = ENOENT;
	return nullptr;
}

Inode* DiskOperations::getInode(const char* path, int pathLen, int &error)
{
	int lastMemberLen = getLastMemberLen(path, pathLen);

std::cout << "GET INODE: " << path << " len: " << pathLen << " lastmemb: " << lastMemberLen << std::endl;

	if (pathLen == 1 && path[0] == '/')
		return getInodeById(0);

	if (pathLen == 1 && path[0] != '/')
	{
		error = ENOTDIR;
		return nullptr;
	}

	Inode* parent = getParent(path, pathLen, error);

	if (parent == nullptr)
		return nullptr;

	if (parent->fileType != Inode::IT_DIRECTORY)
	{
		error = ENOTDIR;
		return nullptr;
	}

	return getMember(parent, path + pathLen - lastMemberLen, lastMemberLen, error);
}

Inode* DiskOperations::getParent(const char* path, int pathLen, int &error)
{
	int lastMemberLen = getLastMemberLen(path, pathLen);

std::cout << "GET PARENT: " << path << " len: " << pathLen << " lastmemb: " << lastMemberLen << std::endl;

	if (pathLen - lastMemberLen == 1 && path[0] == '/')
		return getInodeById(0);

	if (lastMemberLen == pathLen)
	{
		error = ENOTDIR;
		return nullptr;
	}

	Inode* parent = getInode(path, pathLen - lastMemberLen, error);

	if (parent == nullptr)
		return nullptr;

	if (parent->fileType != Inode::IT_DIRECTORY)
	{
		error = ENOTDIR;
		return nullptr;
	}

	return parent;
}

Inode* DiskOperations::dirNavigate(const char* path, int& error)
{
	size_t pathLen = strlen(path);

	return getInode(path, pathLen, error);
}


Inode* DiskOperations::createInode(const char *path, int mode, int inodeFileType, int& error)
{
	int pathLen = strlen(path);

	Inode* parent = getParent(path, pathLen, error);
	if (parent == nullptr)
	{
std::cout << "PARENT NOT FOUND\n";
		return nullptr;
	}

	const char* name = path + pathLen - getLastMemberLen(path, pathLen);

	if (getMember(parent, name, getLastMemberLen(path, pathLen), error) != nullptr)
	{
std::cout << "ALREADY EXISTS\n";
		error = EEXIST;
		return nullptr;
	}

	if (error != ENOENT)
	{
		return nullptr;
	}

	Directory* parentDir = (Directory*)getShmAddr(parent->blockAddress);
	InodeDirectoryEntry* dirList = parentDir->getInodesArray();
	
	if ((parent->permissions & Inode::PERM_W) == 0)
	{
		error = EACCES;
		return nullptr;
	}

	unsigned int inodeDataBlocks = 1;

	if(inodeFileType == Inode::IT_DIRECTORY)
	{
		inodeDataBlocks = ceil(2 * sizeof(InodeDirectoryEntry) + sizeof(Directory), blockSize);
	}

	usageMap->lock();
	int inodeDataBlockAddress = usageMap->getFreeBlocks(inodeDataBlocks, false);
	if (inodeDataBlockAddress == -1)
	{
		usageMap->unlock();

		error = ENOSPC;
		return nullptr;

	}
	usageMap->markBlocks(inodeDataBlockAddress, inodeDataBlockAddress+inodeDataBlocks, false, false);
	usageMap->unlock();
	
	Inode* inode = getFreeInodeAddr();
	createNewInodeEntry(parent->id, inode, inodeDataBlockAddress, mode, inodeFileType);
	
	parentDir = (Directory *)reallocate(parent, parentDir->getSize() + sizeof(InodeDirectoryEntry));
	if (parentDir != nullptr)
	{
		parentDir->addEntry(InodeDirectoryEntry(inode->id, name));
		parent->modificationDate = time(0);
	}

	return inode;
}
Packet* DiskOperations::mkdir(const char* path, int permissions)
{
	int error;
	sem_wait(&inodeOpSemaphore);
	Inode* ret = createInode(path, permissions, Inode::IT_DIRECTORY, error);
	sem_post(&inodeOpSemaphore);
	if(ret == nullptr)
		return new ErrorResponse(error);

	return new OKResponse();
}

Packet *DiskOperations::lseek(FileDescriptor *fd, int offset, int whence)
{
	if (whence == SEEK_SET)
	{
		if (offset < 0 || offset > fd->inode->nodeSize)
		{
			return new ErrorResponse(EINVAL);
		}
		fd->position = (unsigned long long)offset;
	}
	else if (whence == SEEK_CUR || whence == SEEK_END)
	{
		unsigned long long newPosition;
		if (whence == SEEK_CUR)
			newPosition = fd->position + offset;
		else
			newPosition = fd->inode->nodeSize + offset;
		if (newPosition < 0 || newPosition > fd->inode->nodeSize)
			return new ErrorResponse(EINVAL);

		fd->position = newPosition;
	}
	else
		return new ErrorResponse(EINVAL);
	return new OKResponse;
}
Packet* DiskOperations::create(const char* path, int mode, int pid)
{
	// mode is not used here.
	return open(path, O_CREAT | O_WRONLY, pid);
}
Packet* DiskOperations::open(const char* path, int flags, int pid)
{
	return openUsingFileDescriptorFlags(path, linuxIntoFileDescriptorFlags(flags), pid);
}
Packet* DiskOperations::openUsingFileDescriptorFlags(const char* path, int flags, int pid)
{
	int error;
std::cout << "OPENING: " << path;

	const char* name = path + strlen(path) - getLastMemberLen(path, strlen(path));
	if ((strcmp(name, ".") == 0 || strcmp(name, "..") == 0 || strcmp(name, "/") == 0) && flags & FileDescriptor::M_CREATE)
	{
		return new ErrorResponse(EEXIST);
	}	

	sem_wait(&inodeOpSemaphore);
	Inode* inodeToOpen = dirNavigate(path, error);

	if(inodeToOpen == nullptr) // eventually creating a file
	{
		if(error == ENOENT && flags & FileDescriptor::M_CREATE)
		{
			unsigned int permissions = 0;
			if(flags & FileDescriptor::M_CREATE_PERM_R) permissions |= Inode::PERM_R;
			if(flags & FileDescriptor::M_CREATE_PERM_W) permissions |= Inode::PERM_W;
			if(flags & FileDescriptor::M_CREATE_PERM_X) permissions |= Inode::PERM_X;
			inodeToOpen = createInode(path, permissions, Inode::IT_FILE, error);

			if(inodeToOpen == nullptr)
			{
				sem_post(&inodeOpSemaphore);
				return new ErrorResponse(error);
			}
		}
		else
		{
			sem_post(&inodeOpSemaphore);
			return new ErrorResponse(error);
		}
	}

	if(inodeToOpen->fileType == Inode::IT_DIRECTORY && flags != FileDescriptor::M_READ)
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(EISDIR);
	}

	if (flags & FileDescriptor::M_READ && !(inodeToOpen->permissions & Inode::PERM_R) ||
		flags & FileDescriptor::M_WRITE && !(inodeToOpen->permissions & Inode::PERM_W))
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(EACCES);
	}

	if((flags & (FileDescriptor::M_READ | FileDescriptor::M_WRITE)) == (FileDescriptor::M_READ | FileDescriptor::M_WRITE)) // RW Access
	{
std::cout <<"OPENING RW\n";
		if(fdTable->inodeStatusMap.OpenForReadWrite(inodeToOpen))
		{
			sem_post(&inodeOpSemaphore);
			return new ErrorResponse(errno);
		}
	}
	if(flags & FileDescriptor::M_READ) // Ronly Access
	{
std::cout <<"OPENING R\n";
		if(fdTable->inodeStatusMap.OpenForReading(inodeToOpen))
		{
			sem_post(&inodeOpSemaphore);
			return new ErrorResponse(errno);
		}
	}
	if(flags & FileDescriptor::M_WRITE) // Wonly Access
	{
std::cout <<"OPENING W\n";
		if(fdTable->inodeStatusMap.OpenForWriting(inodeToOpen))
		{
			sem_post(&inodeOpSemaphore);
			return new ErrorResponse(errno);
		}
	}

	FileDescriptor* fd = fdTable->CreateDescriptor(pid, inodeToOpen, flags);
std::cout << "CREAT_DESC, OFFSET: " << fd->position << "\n";
	FDResponse* fdres = new FDResponse();
	fdres->setFD(fd->number);

	sem_post(&inodeOpSemaphore);
	return fdres;
}

Packet* DiskOperations::unlink(const char* path)
{
	int len = strlen(path);

	if (len == 0 || path[0] != '/')
		return new ErrorResponse(ENOTDIR);

	if (len == 1)
		return new ErrorResponse(EBUSY);

	if (path[len - 1] == '.' && path[len - 2] == '/')
		return new ErrorResponse(EINVAL);

	if (len > 2 && path[len - 1] == '.' && path[len - 2] == '.' && path[len - 3] == '/')
		return new ErrorResponse(ENOTEMPTY);

	int error;

	sem_wait(&inodeOpSemaphore);
	Inode* parent = getParent(path, len, error);

	if (parent == nullptr)
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(error);
	}

	if (!(parent->permissions & Inode::PERM_W))
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(EPERM);
	}

	int removedNameLen = getLastMemberLen(path, len);
	Inode* removed = getMember(parent, path + len - removedNameLen, removedNameLen, error);

	if(removed == nullptr)
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(error);
	}

	if(removed->fileType == Inode::IT_DIRECTORY && reinterpret_cast<Directory*>(getShmAddr(removed->blockAddress))->inodesCount > 2)
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(ENOTEMPTY);
	}

	if (fdTable->inodeStatusMap.InodeStatus(removed) != 0)
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(EBUSY);
	}

	Directory* parentDir = (Directory*)getShmAddr(parent->blockAddress);

	parentDir->deleteEntry(removed->id);
	reallocate(parent, parentDir->getSize());

	int block = removed->blockAddress;
	int removedSize = removed->nodeSize;

	inodeList->deleteInodeEntry(removed->id);

	usageMap->markBlocks(block, block + ceil(removedSize, blockSize), true, true);

	sem_post(&inodeOpSemaphore);
	return new OKResponse;
}

#include <iostream>

Packet* DiskOperations::read(FileDescriptor* fd, int len)
{
	if (fd->mode & FileDescriptor::M_READ)
		return new ErrorResponse(EBADF);
		
std::cout << "READ_FUNC\n";
	int bytesAvailable = std::max((int32_t)0,(int32_t)fd->inode->nodeSize-(int32_t)fd->position);
std::cout << "Bytes avail. " << fd->inode->nodeSize << " " << fd->position << " " << len << std::endl;
	int bytesRead = std::min(bytesAvailable, len);

	fd->inode->accessDate = time(0);

	ShmemPtr shmemPtr;
	shmemPtr.shmid = shmid;
	shmemPtr.offset = (unsigned int)(fd->inode->blockAddress*blockSize + fd->position);
	shmemPtr.size = (unsigned int)bytesRead;

	ShmemPtrResponse* shmemPtrResponse = new ShmemPtrResponse;
	shmemPtrResponse->setPtr(shmemPtr);

std::cout << "BytesRead: " << bytesRead << std::endl;
	fd->position += bytesRead;

	return shmemPtrResponse;
}

Packet* DiskOperations::write(FileDescriptor* fd, int len)
{
	if (fd->mode & FileDescriptor::M_WRITE == 0)
		return new ErrorResponse(EBADF);

	int newPos = fd->position + len;

	unsigned char* shmempos = reallocate(fd->inode, newPos);

	if (shmempos == nullptr)
		return new ErrorResponse(EFBIG);

	ShmemPtr ptr = ShmemPtr{.shmid=shmid, .offset=fd->inode->blockAddress*blockSize + fd->position, .size=len};
	ShmemPtrResponse* response = new ShmemPtrResponse;

	response->setPtr(ptr);

	fd->inode->modificationDate = time(0);
	fd->inode->accessDate = time(0);
	fd->position = newPos;

	return response;
}

Packet* DiskOperations::chmod(const char* path, int mode)
{
	int error;
	sem_wait(&inodeOpSemaphore);

	Inode* inodeToOpen = dirNavigate(path, error);
	if(inodeToOpen == nullptr) // error
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(error);
	}

	if(fdTable->inodeStatusMap.InodeStatus(inodeToOpen) != 0)
	{
		sem_post(&inodeOpSemaphore);
		return new ErrorResponse(EACCES);
	}

	inodeToOpen->permissions = 0;

	if(mode & S_IROTH) inodeToOpen->permissions |= Inode::PERM_R;
	if(mode & S_IWOTH) inodeToOpen->permissions |= Inode::PERM_W;
	if(mode & S_IXOTH) inodeToOpen->permissions |= Inode::PERM_X;

	sem_post(&inodeOpSemaphore);
	return new OKResponse();
}

int DiskOperations::linuxIntoFileDescriptorFlags(int flags)
{
	int outmode = 0;

	if(flags & O_WRONLY) outmode |= FileDescriptor::M_WRITE;
	else if(flags & O_RDWR) outmode |= FileDescriptor::M_WRITE | FileDescriptor::M_READ;
	else outmode |= FileDescriptor::M_READ;

	if(flags & O_CREAT)
	{
		// let's permissions just be RW
		outmode |= FileDescriptor::M_CREATE;
		outmode |= FileDescriptor::M_CREATE_PERM_R;
		outmode |= FileDescriptor::M_CREATE_PERM_W;
	}

	return outmode;
}
