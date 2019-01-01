#pragma once

#include <sys/stat.h>

#ifdef C_PLUS_PLUS
namespace simplefs
{
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C int simplefs_open(const char* path, int mode);
EXTERN_C int simplefs_unlink(const char* path);
EXTERN_C int simplefs_mkdir(const char* path, int mode);
EXTERN_C int simplefs_create(const char* path, int mode);
EXTERN_C int simplefs_read(int fd, char* buf, int len);
EXTERN_C int simplefs_write(int fd, const char* buf, int len);
EXTERN_C int simplefs_lseek(int fd, int offset, int whence);
EXTERN_C int simplefs_chmode(const char* path, mode_t mode);
EXTERN_C int simplefs_close(int fd);

#ifdef C_PLUS_PLUS
}
#endif

