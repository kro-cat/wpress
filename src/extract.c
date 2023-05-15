#include <config.h>

#include <errno.h>
//#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h> // mkdir()

#include "global.h"
#include "mode.h"
#include "header.h"
#include "extract.h"
#include "filesystem.h"

int mkdir_p(char *path)
{
	char *p;

	// TODO: make sure this is portable to other operating systems.
	//       (it probably isn't)
	// FIXME: program will break later on if filename (not path)
	//        has directory separators in it.

	if (path == NULL) {
		errno = EINVAL;
		PRINT_ERROR("%s\n", strerror(errno));
		return -1;
	}

	PRINT_INFO("mkdir -p \"%s\"\n", path);

	/* replace directory separators */
	// TODO: make each directory in order eg: /usr -> /usr/local
	p = strchr(path, PATH_SEPARATOR);
	do {
		*p = 0;
		PRINT_DEBUG("mkdir(\"%s\")\n", path);
		if (mkdir(path, 0755)) {
			if (errno != EEXIST) {
				free(path);
				PRINT_ERROR("%s\n", strerror(errno));
				return -1;
			}
		}
		*(p++) = PATH_SEPARATOR;
	} while ((p = strchr(p, PATH_SEPARATOR)));

	return 0;
}

#define BLOCK_DEFAULT_SIZE 1024

int write_file(char *path, size_t size, FILE *fp_in)
{
	FILE *fp_out;
	int ret;
	void *block;
	size_t remaining_size, block_size;

	if ((path == NULL) || (fp_in == NULL)) {
		errno = EINVAL;
		PRINT_ERROR("%s\n", strerror(errno));
		return -1;
	}

	if ((block = malloc(BLOCK_DEFAULT_SIZE)) == NULL) {
		PRINT_ERROR("%s\n", strerror(errno));
		return -1;
	}

	PRINT_INFO("writing to file: %s\n", path);

	/* make full path */
	if (mkdir_p(path))
		return -1;

	// TODO: don't overwrite a NEWER file
	/* open file for writing, truncate */
	if ((fp_out = fopen(path, "wb")) == NULL) {
		PRINT_ERROR("%s\n", strerror(errno));
		return -1;
	}

	// TODO: keep track of multi-page files
	remaining_size = size;

	while (remaining_size) {
		block_size = (remaining_size > BLOCK_DEFAULT_SIZE) ?
			BLOCK_DEFAULT_SIZE : remaining_size;

		ret = fread(block, 1, block_size, fp_in);
		if (!ret) {
			if (ferror(fp_in)) {
				PRINT_ERROR("%s\n", strerror(errno));
				goto die;
			}
			if (feof(fp_in)) {
				errno = ENODATA;
				PRINT_ERROR("%s\n", strerror(errno));
				goto die;
			}
		}

		block_size = ret;

		/* pre-emptively calculate remaining size */
		remaining_size -= block_size;

		/* write data to file */
		// FIXME: infinite loop if writes blocked
		while (block_size) {
			ret = fwrite(block, 1, block_size, fp_out);
			if (!ret && ferror(fp_out)) {
				PRINT_ERROR("%s\n", strerror(errno));
				goto die;
			}

			block_size -= ret;
		}
	}

	fclose(fp_out);
	free(block);
	return 0;
die:
	fclose(fp_out);
	free(block);
	return -1;
}

int wpress_extract(modeflags_t *flags, FILE *fp_in)
{
	char *path;
	header_t hdr;

	// TODO: option to choose output directory, b/c is just the cwd atm
	// TODO: option to choose specific files

	if (!flags->absolute_names)
		puts("stripping leading '/' from file names");

	while(1) {
		if ((hdr = header_from_file(fp_in)) == NULL)
			goto die_no_header;

		if (eof_header(hdr))
			break;

		path = hdr->path;
		/* strip leading '/' unless --absolute-names */
		if ((!flags->absolute_names) && (path[0] == PATH_SEPARATOR))
			path++;

		if (flags->verbose) {
			printf("%ld %s\n", hdr->size, path);
		} else if (flags->list_only) {
			puts(path);
		}

		if (!flags->list_only) {
			if (write_file(path, hdr->size, fp_in))
				goto die;
		} else {
			/* throwaway contents */
			while (hdr->size--)
				fgetc(fp_in);
		}

		free_header(hdr);

		// TODO: loop until eof I guess
		//break; // for testing.
	}

	return 0;
die:
	free_header(hdr);
die_no_header:
	return -1;
}
