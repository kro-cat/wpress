#ifndef EXTRACT_H
#define EXTRACT_H

#include "mode.h"

#define BLOCK_DEFAULT_SIZE 1024

int wpress_extract(modeflags_t *flags, FILE *fp_in);
int wpress_list(modeflags_t *flags, FILE *fp_in);

#endif
