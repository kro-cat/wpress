#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include "mode.h"

void usage();

int parse_args(int argc, char * const *argv, modes *mode, modeflags_t *flags,
	       FILE **fp_in);

#endif
