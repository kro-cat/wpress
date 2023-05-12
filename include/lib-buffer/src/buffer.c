#include <config.h>

#include <errno.h>
#include <fcntl.h> /* O_RDONLY, O_WRONLY, O_RDWR, etc. */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/stat.h>

#include "buffer.h"

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

int _buffer_seek(buffer_t buffer, off_t offset, int whence)
{
	if (whence == SEEK_CUR) {
		offset = buffer->offset + offset;
		whence = SEEK_SET;
	}

	if ((offset = _lseek(buffer->filedes, offset, whence)) == -1)
		return -1;

	buffer->offset = offset;

	return 0;
}

int _buffer_read(buffer_t buffer)
{
	ssize_t bytes_read;

	if (buffer->buf.ptr == NULL) /* nothing to read, fail silently. */
		return 0;

	/* fill all of the buffer with zeroes */
	memset(buffer->buf.ptr, 0, buffer->buf.size);

	if ((bytes_read = _read(buffer->filedes, buffer->buf.ptr,
				buffer->buf.size)) == -1)
		return -1;

	buffer->datalength = bytes_read;

	return 0;
}

int _buffer_write(buffer_t buffer)
{
	ssize_t bytes_wrote;

	if ((bytes_wrote = _write(buffer->filedes, buffer->buf.ptr,
				   buffer->datalength)) == -1)
		return -1;

	/* move remaining data to the front of the buffer, if any */
	if (bytes_wrote < buffer->datalength)
		memmove(buffer->buf.ptr, buffer->buf.ptr + bytes_wrote,
			buffer->datalength - bytes_wrote);

	/* fill the rest of the buffer with zeroes */
	memset(buffer->buf.ptr + bytes_wrote, 0,
	       buffer->datalength - bytes_wrote);

	buffer->offset += bytes_wrote;

	return bytes_wrote;
}

buffer_t buffer_init()
{
	buffer_t buffer = malloc(sizeof(struct buffer));

	buffer->datalength = 0;
	buffer->buf.ptr = NULL;
	buffer->buf.size = 0;
	buffer->filedes = -1;
	//buffer->read = 0;

	return buffer;
}

void buffer_free(buffer_t buffer)
{
	if (!buffer)
		return;

	if (buffer->filedes != -1)
		buffer_close(buffer);

	free(buffer);
}

int buffer_resize(buffer_t buffer, size_t size)
{
	void *tmp;

	tmp = realloc(buffer->buf.ptr, size);

	if ((tmp == NULL) && (size != 0)) {
		errno = ENOMEM;
		return -1;
	}

	buffer->buf.ptr = tmp;
	buffer->buf.size = size;

	return 0;
}

int buffer_set_datalength(buffer_t buffer, size_t datalength)
{
	if (datalength > buffer->buf.size) {
		errno = ENOMEM;
		return -1;
	}

	buffer->datalength = datalength;

	return 0;
}

size_t buffer_get_datalength(buffer_t buffer)
{
	return buffer->datalength;
}

/**
 * open a file readonly (for rbuf functions)
 */
int buffer_open(buffer_t buffer, const char *path, int oflag)
{
#if defined(OS_LINUX)
	if ((buffer->filedes = _open(path, oflag, S_IRUSR | S_IWUSR)) == -1)
#else
	if ((buffer->filedes =
		     _open(path, oflag | _O_BINARY, S_IRUSR | S_IWUSR)) == -1)
#endif
		return -1;
	return 0;
}

/**
 * close a previously opened file
 */
int buffer_close(buffer_t buffer)
{
	free(buffer->buf.ptr);
	buffer->buf.ptr = NULL;

	if (_close(buffer->filedes))
		return -1;

	buffer->filedes = -1;
}

off_t buffer_get_filesize(buffer_t buffer)
{
	struct stat buf;

	if (fstat(buffer->filedes, &buf))
		return -1;

	return buf.st_size;
}

off_t buffer_get_offset(buffer_t buffer)
{
	return buffer->offset;
}

int buffer_eof(buffer_t buffer)
{
	/* include this function b/c the following operation is not really
	 * all that obvious. This doesn't always work, like if the io operation
	 * was interrupted, but maybe I'll make it more robust in the future. */
	return buffer->datalength != buffer->buf.size;
}

int buffer_seek(buffer_t buffer, off_t offset, int whence)
{
	/* seek */
	if (_buffer_seek(buffer, offset, whence))
		return -1;

	/* read */
	return _buffer_read(buffer);
}

int buffer_rewind(buffer_t buffer)
{
	return buffer_seek(buffer, 0, SEEK_SET);
}

int buffer_reload(buffer_t buffer)
{
	return buffer_seek(buffer, 0, SEEK_CUR);
}

int buffer_write(buffer_t buffer)
{
	/* seek to current offset in file */
	if (_buffer_seek(buffer, 0, SEEK_CUR))
		return -1;

	/* write */
	return _buffer_write(buffer);
}

buffer_t buffer(const char *path, int oflag) {
	buffer_t buffer = buffer_init();

	buffer_open(buffer, path, oflag);

	if (buffer_resize(buffer, BUFFER_DEFAULT_SIZE)) {
		buffer_free(buffer);
		return NULL;
	}

	/* ALWAYS initialize this memory */
	memset(buffer->buf.ptr, 0, buffer->buf.size);

	if (_buffer_seek(buffer, 0, SEEK_SET)) {
		buffer_free(buffer);
		return NULL;
	}
}
