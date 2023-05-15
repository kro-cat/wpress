#ifndef MODE_H
#define MODE_H

typedef enum { MODE_DEFAULT=0, MODE_EXTRACT=1 } modes;

typedef struct modeflags {
	int list_only;
	int absolute_names;
	int verbose;
} modeflags_t;

#endif
