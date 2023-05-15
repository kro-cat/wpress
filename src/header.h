#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>

typedef struct header {
	char *path;
	size_t size;
	//time_t mtime;
} *header_t;

int eof_header(header_t hdr);

header_t header_from_file(FILE *fp);
header_t header();
void free_header(header_t hdr);

#endif
