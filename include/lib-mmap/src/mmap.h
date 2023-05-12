#ifndef BUFFER_H
#define BUFFER_H

typedef struct mmap *mmap_t;

/**
 * construct a new mmap structure to be "nothing"
 * @param mmap  [description]
 */
mmap_t mmap_init();

void mmap_free(mmap_t mmap);

/**
 * resize internal mmap
 * @param  mmap               [description]
 * @param  size                 [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int mmap_resize(mmap_t mmap, size_t size);

int mmap_get_filedes(mmap_t mmap);
off_t mmap_get_filesize(mmap_t mmap);
void *mmap_get_bufptr(mmap_t mmap);
size_t mmap_get_bufsize(mmap_t mmap);
int mmap_set_datalength(mmap_t mmap, size_t datalength);
size_t mmap_get_datalength(mmap_t mmap);

/**
 * open a file readonly (for rbuf functions)
 * @param  mmap               [description]
 * @param  path                 path to file
 * @return        0 on success, -1 on failure (see errno)
 */
int mmap_open(mmap_t mmap, const char *path, int oflag);

/**
 * close a previously opened file
 * @param  mmap               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int mmap_close(mmap_t mmap);

/**
 * get position in current file
 * @param  mmap               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
off_t mmap_get_offset(mmap_t mmap);

/**
 * true if we hit EOF in file, not the mmap. sometimes lies about this.
 * @param mmap		[description]
 * @return	0 if not EOF, nonzero otherwise.
 */
int mmap_eof(mmap_t mmap);

/**
 * set the position of the frame within the current file. doesn't refresh mmap.
 * @param  mmap               [description]
 * @param  offset               [description]
 * @param  whence               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
int mmap_seek(mmap_t mmap, off_t offset, int whence);

/**
 * a more intuitive way of seeking to the start of the file
 * @param  mmap               [description]
 * @return        positive offset from the beginning of the file, -1 on failure
 */
int mmap_rewind(mmap_t mmap);

/**
 * reload the current frame at the current position (doesn't modify file offset)
 * @param  mmap               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int mmap_reload(mmap_t mmap);

/**
 * commit the current frame at the real file offset (modifies file offset and st_size, no seek)
 * Acts like a pager
 * @param  mmap               [description]
 * @return        0 on success, -1 on failure (see errno)
 */
int mmap_write(mmap_t mmap);

#endif

