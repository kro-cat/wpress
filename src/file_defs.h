#ifndef FILE_DEFS_H
#define FILE_DEFS_H

#if defined(WIN32) || defined(_WIN32) || defined(OS_WINDOWS)
#include <io.h> /* low level file stuff */
#define BAD_PATH_SEPARATOR '/'
#define PATH_SEPARATOR '\\'
#define O_BINARY _O_BINARY
#elif defined(OS_LINUX)
#include <unistd.h> /* low level file stuff */
#define BAD_PATH_SEPARATOR '\\'
#define PATH_SEPARATOR '/'
#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
#define _O_BINARY 0
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_RDONLY O_RDONLY
#define _O_RDWR O_RDWR
#define _O_WRONLY O_WRONLY
#else
#error "This library will not build."
#endif

#endif /* FILE_DEFS_H */
