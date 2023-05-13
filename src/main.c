#include <config.h>

#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h> // mkdir()

#include "global.h"
#include "file_defs.h"


#define NAME_SIZE 255
#define SIZE_SIZE 14
#define MTIME_SIZE 12
#define PREFIX_SIZE 4096

#define NAME_OFFSET 0
#define SIZE_OFFSET 255
#define MTIME_OFFSET 269
#define PREFIX_OFFSET 281

typedef struct __attribute__((packed)) header {
	char name[NAME_SIZE];
	char size[SIZE_SIZE];
	char mtime[MTIME_SIZE];
	char prefix[PREFIX_SIZE];
} header_t;

#define HEADER_SIZE sizeof(struct header)

int is_eof_header(header_t *phdr)
{
	if (phdr->size[0] != 0)
		return 0;
	if (phdr->mtime[0] != 0)
		return 0;
	if (phdr->name[0] != 0)
		return 0;
	if (phdr->prefix[0] != 0)
		return 0;
	return 1;
}

char *make_fullpath(header_t *phdr, int absolute_names)
{
	char *p;
	char *path;
	size_t span;
	size_t prefix_length = PREFIX_SIZE;
	size_t name_length = NAME_SIZE;

	// TODO: make sure this is portable to other operating systems.
	//       (it probably isn't)
	// FIXME: program will break later on if filename (not path)
	//        has directory separators in it.

	if (phdr->prefix[PREFIX_SIZE - 1] == 0)
		prefix_length = strlen(phdr->prefix);
	if (phdr->name[NAME_SIZE - 1] == 0)
		name_length = strlen(phdr->name);

	path = malloc(prefix_length + name_length + 2);

	if (path == NULL) {
		PRINT_ERROR("malloc(): %s\n", strerror(errno));
		return NULL;
	}

	if (phdr->prefix[0] != 0) {
		if (!absolute_names) {
			if ((phdr->prefix[0] == BAD_PATH_SEPARATOR)
			    || (phdr->prefix[0] == PATH_SEPARATOR))
				memcpy(path, phdr->prefix + 1, --prefix_length);
		} else {
			memcpy(path, phdr->prefix, prefix_length);
		}

		path[prefix_length] = 0;
		path[prefix_length + 1] = 0;

		PRINT_INFO("mkdir(\"%s\")\n", path);

		if (prefix_length) {
			/* replace directory separators */
			// TODO: make each directory in order eg: /usr -> /usr/local
			p = path;
			while ((span = strcspn(p, "\\/"))) {
				p += span;
				*p = 0;
				PRINT_INFO("mkdir(\"%s\")\n", path);
				if (mkdir(path, 0755)) {
					if (errno != EEXIST) {
						free(path);
						PRINT_ERROR("mkdir(): %s\n",
							    strerror(errno));
						return NULL;
					}
				}
				*(p++) = PATH_SEPARATOR;
			}
			*p = 0;
		}
	}

	/* append filename to path */
	strncat(path, phdr->name, name_length);
	return path;
}

#define BLOCK_DEFAULT_SIZE 1024

int write_file(FILE *fp_out, size_t size, FILE *fp_in)
{
	int ret;
	void *block;
	size_t remaining_size, block_size;

	if ((block = malloc(BLOCK_DEFAULT_SIZE)) == NULL) {
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

	free(block);
	return 0;
die:
	free(block);
	return -1;
}

int wpress_extract(const char *path, int absolute_names)
{
	int ret;
	long unsigned int bytes_read;
	FILE *fp_in;
	FILE *fp_out;
	char *outfile_path;
	header_t *phdr;

	if ((phdr = malloc(HEADER_SIZE)) == NULL) {
		PRINT_ERROR("%s\n", strerror(errno));
		return -1;
	}

	if (strcmp(path, "-")) {
		if ((fp_in = fopen(path, "rb")) == NULL) {
			PRINT_ERROR("%s\n", strerror(errno));
			goto die_no_file;
		}
	} else {
		fp_in = stdin;
	}

	// TODO: option to choose output directory, b/c is just the cwd atm
	// TODO: option to choose specific files

	while(1) {
		bytes_read = 0;
		while (bytes_read < HEADER_SIZE) {
			ret = fread(phdr + bytes_read, 1, HEADER_SIZE, fp_in);
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
			bytes_read += ret;
		}

		if (is_eof_header(phdr))
			break;

		if ((phdr == NULL) || (phdr->name[0] == 0)) {
			errno = EINVAL;
			PRINT_ERROR("%s\n", strerror(errno));
			return -1;
		}

		/* ensure path */
		if ((outfile_path = make_fullpath(phdr, absolute_names)) == NULL)
			goto die_no_file;

		// TODO: don't overwrite a NEWER file
		PRINT_INFO("writing to file: %s\n", outfile_path);

		/* open file for writing, truncate */
		if ((fp_out = fopen(outfile_path, "wb")) == NULL) {
			PRINT_ERROR("%s\n", strerror(errno));
			free(outfile_path);
			goto die_no_file;
		}

		free(outfile_path);

		if (write_file(fp_out, strtoull(phdr->size, NULL, 10), fp_in)) {
			fclose(fp_out);
			goto die;
		}

		fclose(fp_out);

		// TODO: loop until eof I guess
		//break; // for testing.
	}

	if (fp_in != stdin)
		fclose(fp_in);
	free(phdr);
	return 0;
die:
	if (fp_in != stdin)
		fclose(fp_in);
die_no_file:
	free(phdr);
	return -1;
}

void usage()
{
	// TODO: accept option to extract specific files:
	// "wpress [OPTION...] [FILE]..." - extract FILE(s) from archive
	puts("\
Usage: " S(EXECNAME) " [OPTION...]\n\
Extract all files from a \"wpress\" archive.\n\
\n\
Options:\n\
  -h, --help                         show this message\n\
  -x, --extract, --get               extract files from an archive\n\
  -f, --file=ARCHIVE                 use archive file or device ARCHIVE\n\
  -P, --absolute-names               don't strip leading '/'s from file names\n");
}

typedef enum { MODE_DEFAULT=0, MODE_EXTRACT=1 } modes;

int parse_args(int argc, char * const *argv, modes *mode, const char **file, int *absolute_names)
{
	int opt;
	char *sp;

	const char *optstring = "hxf:P";
	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "file", required_argument, NULL, 'f' },
		{ "extract", no_argument, NULL, 'x' },
		{ "get", no_argument, NULL, 'x' },
		{ "absolute-names", no_argument, NULL, 'P' },
		{ 0, 0, NULL, 0 }
	};

	if (argv == NULL)
		return -2; // exit 1

	/* optional arguments */
	while ((opt = getopt_long(argc, argv, optstring, longopts, NULL))
	       != -1) {
		switch (opt) {
		case 'f':
			// File
			*file = optarg;
			break;
		case 'h':
			// Help
			usage();
			return -1; // exit 0
		case 'x':
			// Extract
			*mode = MODE_EXTRACT;
			break;
		case 'P':
			*absolute_names = 1;
			break;
		case '?':
			__attribute__((fallthrough));
		default:
			if ((sp = strchr(optstring, optopt)) != NULL) {
				PRINT_ERROR(
					"option '%c' requires and argument\n",
					optopt);
			} else {
				PRINT_ERROR("unknown option '%c'\n", optopt);
			}
			usage();
			return -3; // exit 2
		}
	}

	// TODO: accept files to extract as positional arguments
	/* positional arguments */
	while (optind < argc) {
		//if (my arg is not empty) {
		//	set my arg from positional argument
		//} else {
		//	fail miserably
		PRINT_ERROR("no candidate for positional argument '%s'\n",
			    optarg);
		usage();
		return -3; // exit 2
		//}
	}

	return 0;
}

int main(int argc, char * const *argv)
{
	int ret = 0;
	modes mode = MODE_DEFAULT;
	const char *file = "-";
	int absolute_names = 0;

	if ((ret = parse_args(argc, argv, &mode, &file, &absolute_names)))
		return ~ret;

	switch (mode) {
	case MODE_DEFAULT:
		puts(S(EXECNAME) ": you should specify \"-x\" for extract.");
		usage();
		return 2;
	case MODE_EXTRACT:
		wpress_extract(file, absolute_names);
	}

	return 0;
}
