#include <config.h>

#include "global.h"
#include "parse_args.h"
#include "extract.h"

int main(int argc, char * const *argv)
{
	int ret = 0;
	FILE *fp_in = stdin;
	modes mode = MODE_DEFAULT;
	modeflags_t flags = {0};

	if ((ret = parse_args(argc, argv, &mode, &flags, &fp_in)))
		goto die;

	switch (mode) {
	case MODE_DEFAULT:
		MESSAGE("you should specify \"-x\" for extract.");
		goto bad_mode;
	case MODE_EXTRACT:
		wpress_extract(&flags, fp_in);
	}

	ret = -1;
die:
	if (fp_in != stdin)
		fclose(fp_in);
	return ~ret;
bad_mode:
	if (fp_in != stdin)
		fclose(fp_in);
	usage();
	return 2;
}
