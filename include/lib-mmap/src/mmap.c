#include <config.h>

#include <errno.h>
#include <fcntl.h> /* O_RDONLY, O_WRONLY, O_RDWR, etc. */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/stat.h>

#include "mmap.h"

#if defined(OS_WINDOWS)
#include <io.h> /* low level file stuff */
#elif defined(OS_LINUX)
#include <unistd.h> /* low level file stuff */
#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
#define _O_RDONLY O_RDONLY
#define _O_RDWR O_RDWR
#define _O_WRONLY O_WRONLY
#else
#error "This library will not build."
#endif

/* defaults - you can override these when building this library */

#ifndef BUFFER_DEFAULT_SIZE
/* default frame size - default 1KiB */
#define BUFFER_DEFAULT_SIZE 1023
#endif

struct mmap {
	size_t filelength; /* size of the file's readable data on disk */
	size_t datalength; /* size of the data in the mmap in bytes */

	struct {
		void *ptr; /* operation mmap */
		size_t size; /* size of the mmap in bytes */
	} buf;

	int filedes; /* filedes number */
	off_t offset; /* offset in file from where the start of mmap is */
	int read;
};

int _mmap_seek(mmap_t mmap, off_t offset, int whence)
{
	if (whence == SEEK_CUR) {
		offset = mmap->offset + offset;
		whence = SEEK_SET;
	}

	if ((offset = _lseek(mmap->filedes, offset, whence)) == -1)
		return -1;

	mmap->offset = offset;

	return 0;
}

int _mmap_read(mmap_t mmap)
{
	ssize_t bytes_read;

	/* fill all of the mmap with zeroes */
	memset(mmap->buf.ptr, 0, mmap->buf.size);

	if ((bytes_read = _read(mmap->filedes, mmap->buf.ptr,
				mmap->buf.size)) == -1)
		return -1;

	mmap->datalength = bytes_read;

	return 0;
}

int _mmap_write(mmap_t mmap)
{
	ssize_t bytes_wrote;

	if ((bytes_wrote = _write(mmap->filedes, mmap->buf.ptr,
				  mmap->datalength)) == -1)
		return -1;

	/* move remaining data to the front of the mmap, if any */
	if (bytes_wrote < mmap->datalength)
		memmove(mmap->buf.ptr, mmap->buf.ptr + bytes_wrote,
			mmap->datalength - bytes_wrote);

	/* fill the rest of the mmap with zeroes */
	memset(mmap->buf.ptr + bytes_wrote, 0,
	       mmap->datalength - bytes_wrote);

	mmap->offset += bytes_wrote;

	return 0;
}

mmap_t mmap_init()
{
	mmap_t mmap = malloc(sizeof(struct mmap));

	mmap->datalength = 0;
	mmap->buf.ptr = NULL;
	mmap->buf.size = BUFFER_DEFAULT_SIZE;
	mmap->filedes = -1;
	mmap->read = 0;

	return mmap;
}

void mmap_free(mmap_t mmap)
{
	if (!mmap)
		return;

	if (mmap->filedes != -1)
		mmap_close(mmap);

	free(mmap);
}

int mmap_resize(mmap_t mmap, size_t size)
{
	void *tmp;

	tmp = realloc(mmap->buf.ptr, size);

	if ((tmp == NULL) && (size != 0)) {
		errno = ENOMEM;
		return -1;
	}

	mmap->buf.ptr = tmp;
	mmap->buf.size = size;

	return 0;
}

void *mmap_get_bufptr(mmap_t mmap)
{
	return mmap->buf.ptr;
}

size_t mmap_get_bufsize(mmap_t mmap)
{
	return mmap->buf.size;
}

int mmap_set_datalength(mmap_t mmap, size_t datalength)
{
	if (datalength > mmap->buf.size) {
		errno = ENOMEM;
		return -1;
	}

	mmap->datalength = datalength;

	return 0;
}

size_t mmap_get_datalength(mmap_t mmap)
{
	return mmap->datalength;
}

/**
 * open a file readonly (for rbuf functions)
 */
int mmap_open(mmap_t mmap, const char *path, int oflag)
{
#if defined(OS_LINUX)
	if ((mmap->filedes = _open(path, oflag, S_IRUSR | S_IWUSR)) == -1)
#else
	if ((mmap->filedes =
		     _open(path, oflag | _O_BINARY, S_IRUSR | S_IWUSR)) == -1)
#endif
		return -1;

	if (mmap_resize(mmap, mmap->buf.size))
		return -1;

	/* ALWAYS initialize this memory */
	memset(mmap->buf.ptr, 0, mmap->buf.size);

	if (_mmap_seek(mmap, 0, SEEK_SET))
		return -1;

	return 0;
}

/**
 * close a previously opened file
 */
int mmap_close(mmap_t mmap)
{
	free(mmap->buf.ptr);
	mmap->buf.ptr = NULL;

	if (_close(mmap->filedes))
		return -1;

	mmap->filedes = -1;
}

int mmap_get_filedes(mmap_t mmap)
{
	return mmap->filedes;
}

off_t mmap_get_filesize(mmap_t mmap)
{
	struct stat buf;

	if (fstat(mmap->filedes, &buf))
		return -1;

	return buf.st_size;
}

off_t mmap_get_offset(mmap_t mmap)
{
	return mmap->offset;
}

int mmap_eof(mmap_t mmap)
{
	/* include this function b/c the following operation is not really
	 * all that obvious. This doesn't always work, like if the io operation
	 * was interrupted, but maybe I'll make it more robust in the future. */
	return mmap->datalength != mmap->buf.size;
}

int mmap_seek(mmap_t mmap, off_t offset, int whence)
{
	/* seek */
	if (_mmap_seek(mmap, offset, whence))
		return -1;

	/* read */
	return _mmap_read(mmap);
}

int mmap_rewind(mmap_t mmap)
{
	return mmap_seek(mmap, 0, SEEK_SET);
}

int mmap_reload(mmap_t mmap)
{
	return mmap_seek(mmap, 0, SEEK_CUR);
}

int mmap_write(mmap_t mmap)
{
	/* seek */
	if (_mmap_seek(mmap, 0, SEEK_CUR))
		return -1;

	/* write */
	return _mmap_write(mmap);
}

