#include <config.h>

#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "global.h"
#include "mmap.h"

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
    -f, --file=ARCHIVE                 use archive file or device ARCHIVE\n");
}

typedef enum { MODE_DEFAULT=0, MODE_EXTRACT=1 } modes;

int parse_args(int argc, char * const *argv, modes *mode)
{
	int opt;
	char *sp;

	char *file = "-";
	const char *optstring = "hxf:";
	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "file", required_argument, NULL, 'f' },
		{ "extract", no_argument, NULL, 'x' },
		{ "get", no_argument, NULL, 'x' },
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
			file = optarg;
			break;
		case 'h':
			// Help
			usage();
			return -1; // exit 0
		case 'x':
			// Extract
			*mode = MODE_EXTRACT;
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

	if ((ret = parse_args(argc, argv, &mode)))
		return ~ret;

	return 0;
}
