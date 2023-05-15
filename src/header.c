#include <config.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "header.h"
#include "filesystem.h"

#define NAME_SIZE 255
#define SIZE_SIZE 14
#define MTIME_SIZE 12
#define PREFIX_SIZE 4096

#define NAME_OFFSET 0
#define SIZE_OFFSET 255
#define MTIME_OFFSET 269
#define PREFIX_OFFSET 281

typedef struct __attribute__((packed)) realheader {
	char name[NAME_SIZE];
	char size[SIZE_SIZE];
	char mtime[MTIME_SIZE];
	char prefix[PREFIX_SIZE];
} realheader_t;

#define HEADER_SIZE sizeof(struct realheader)

int eof_header(header_t hdr)
{
	if (hdr == NULL)
		return 0;

	if (hdr->size || hdr->path)
		return 0;

	return 1;
}

int valid_header(realheader_t hdr)
{
	if (!(hdr.size[0] || hdr.mtime[0] || hdr.name[0] || hdr.prefix[0]))
		return 1;

	if (hdr.size[0] && hdr.mtime[0] && hdr.name[0] && hdr.prefix[0])
		return 2;

	return 0;
}

void free_header(header_t hdr)
{
	if (hdr->path)
		free(hdr->path);
	free(hdr);
}

header_t header()
{
	header_t hdr;

	if ((hdr = malloc(sizeof(struct header))) == NULL)
		return NULL;

	hdr->path = NULL;
	hdr->size = 0;

	return hdr;
}

void sanitize_path(char *path)
{
	char *p = path;

	if (path == NULL)
		return;

	/* if there are only bad path separators, then it's pretty obvious that
	 * we need to translate them. Otherwise, it's kind of subjective. */
	if ((p = strchr(path, BAD_PATH_SEPARATOR))
	    && !(strchr(path, PATH_SEPARATOR))) {
		do {
			*(p++) = PATH_SEPARATOR;
		} while ((p = strchr(p, BAD_PATH_SEPARATOR)));
	}
}

header_t header_from_file(FILE *fp_in)
{
	int ret;
	size_t bytes_read = 0;
	realheader_t raw_hdr = {0};
	size_t prefix_length = PREFIX_SIZE;
	size_t name_length = NAME_SIZE;
	header_t hdr;

	while (bytes_read < HEADER_SIZE) {
		ret = fread(&raw_hdr + bytes_read, 1, HEADER_SIZE, fp_in);
		if (!ret) {
			if (ferror(fp_in))
				return NULL;
			if (feof(fp_in)) {
				errno = ENODATA;
				return NULL;
			}
		}
		bytes_read += ret;
	}

	/* validate header */
	if (!(ret = valid_header(raw_hdr))) {
		errno = EINVAL;
		return NULL;
	}

	if (ret == 1) /* blank header is an eof header */
		return header();

	if (raw_hdr.prefix[PREFIX_SIZE - 1] == 0)
		prefix_length = strlen(raw_hdr.prefix);
	if (raw_hdr.name[NAME_SIZE - 1] == 0)
		name_length = strlen(raw_hdr.name);

	if ((hdr = header()) == NULL)
		return NULL;

	if ((hdr->path = malloc(prefix_length + name_length + 2)) == NULL) {
		free_header(hdr);
		return NULL;
	}

	memcpy(hdr->path, raw_hdr.prefix, prefix_length);
	hdr->path[prefix_length] = 0; // = PATH_SEPARATOR;

	/* we only want to sanitize the prefix part. we're assuming the filename
	 * has no "directory parts" in it. */
	sanitize_path(hdr->path);

	hdr->path[prefix_length] = PATH_SEPARATOR;
	memcpy(hdr->path + prefix_length + 1, raw_hdr.name, name_length);
	hdr->path[name_length + prefix_length + 1] = 0;

	hdr->size = strtoull(raw_hdr.size, NULL, 10);

	return hdr;
}
