#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#if defined(WIN32) || defined(_WIN32) || defined(OS_WINDOWS)
#define BAD_PATH_SEPARATOR '/'
#define PATH_SEPARATOR '\\'
// #include <io.h> /* low level file stuff */
// #define O_BINARY _O_BINARY

#elif defined(OS_LINUX)
#define BAD_PATH_SEPARATOR '\\'
#define PATH_SEPARATOR '/'
// #include <unistd.h> /* low level file stuff */
// #define _open open
// #define _close close
// #define _read read
// #define _write write
// #define _lseek lseek
// #define _O_BINARY 0
// #define _O_CREAT O_CREAT
// #define _O_TRUNC O_TRUNC
// #define _O_RDONLY O_RDONLY
// #define _O_RDWR O_RDWR
// #define _O_WRONLY O_WRONLY

#else
#error "I don't know the path separator for this OS."
#endif

#endif /* FILESYSTEM_H */
