#include <stdio.h>

#ifndef GLOBAL_H
#define GLOBAL_H

#define EXECNAME wpress

#define S(X) _S(X)
#define _S(X) #X

#ifdef DEBUG
#define PRINT_DEBUG(...)\
	fprintf(stderr, "[DEBUG] " S(EXECNAME) " %s: ", __func__);\
	fprintf(stderr, __VA_ARGS__)
#else
#define PRINT_DEBUG(...) /* debug statement removed */
#endif

#define PRINT_ERROR(...)\
	fprintf(stderr, "[ERROR] " S(EXECNAME) " %s: ", __func__);\
	fprintf(stderr, __VA_ARGS__)

#define PRINT_INFO(...)\
	fprintf(stdout, "[INFO] " S(EXECNAME) " %s: ", __func__);\
	fprintf(stdout, __VA_ARGS__)

#endif
