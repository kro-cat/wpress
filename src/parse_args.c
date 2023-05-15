#include <config.h>

#include <errno.h>
#include <getopt.h>
#include <string.h>

#include "global.h"
#include "parse_args.h"

void usage()
{
	// TODO: accept option to extract specific files:
	// "wpress [OPTION...] [FILE]..." - extract FILE(s) from archive
	puts("\
Usage: " S(EXECNAME) " [OPTION...]\n\
Restore files from a \"wpress\" archive.\n\
\n\
Modes: \n\
  -t, --list                         list the contents of an archive\n\
  -x, --extract, --get               extract files from an archive\n\
\n\
Options:\n\
  -f, --file=ARCHIVE                 use archive file or device ARCHIVE\n\
  -h, --help                         show this message\n\
  -P, --absolute-names               don't strip leading '/'s from file names\n\
  -v, --verbose                      verbosely list files processed\n\
");
}

int parse_args(int argc, char * const *argv, modes *mode, modeflags_t *flags,
	       FILE **fp_in)
{
	int opt;
	char *sp;

	const char *optstring = "txf:hPv";
	const struct option longopts[] = {
		{ "list", no_argument, NULL, 't' },
		{ "extract", no_argument, NULL, 'x' },
		{ "get", no_argument, NULL, 'x' },
		{ "file", required_argument, NULL, 'f' },
		{ "help", no_argument, NULL, 'h' },
		{ "absolute-names", no_argument, NULL, 'P' },
		{ "verbose", no_argument, NULL, 'V' },
		{ 0, 0, NULL, 0 }
	};

	if (argv == NULL)
		return -2; // exit 1

	/* optional arguments */
	while ((opt = getopt_long(argc, argv, optstring, longopts, NULL))
	       != -1) {
		switch (opt) {
		case 't':
			// List
			flags->list_only = 1;
			__attribute__((fallthrough));
		case 'x':
			// Extract
			*mode = MODE_EXTRACT;
			break;
		case 'f':
			// File
			if (*fp_in != stdin) {
				PRINT_ERROR(
					"You may only specify one input file.");
				usage();
				return -3;
			}
			if (strcmp(optarg, "-")
			    && ((*fp_in = fopen(optarg, "rb")) == NULL)) {
				PRINT_ERROR("%s\n", strerror(errno));
				return -2;
			}
			break;
		case 'h':
			// Help
			usage();
			return -1; // exit 0
		case 'P':
			flags->absolute_names = 1;
			break;
		case 'v':
			flags->verbose = 1;
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
			    argv[optind]);
		usage();
		return -3; // exit 2
		//}
		//optind++;
	}

	return 0;
}
