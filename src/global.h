#include <stdio.h>

#ifndef GLOBAL_H
#define GLOBAL_H

#define EXECNAME wpress

#define S(X) _S(X)
#define _S(X) #X

#ifdef DEBUG
#define PRINT_DEBUG(...) {\
	fprintf(stderr, S(EXECNAME) ": [DEBUG] in %s(): ", __func__);\
	fprintf(stderr, __VA_ARGS__);}
#else
#define PRINT_DEBUG(...) /* debug statement removed */
#endif

#define PRINT_ERROR(...) {\
	fprintf(stderr, S(EXECNAME) ": [ERROR] in %s(): ", __func__);\
	fprintf(stderr, __VA_ARGS__);}

#define PRINT_INFO(...) {\
	fprintf(stdout, S(EXECNAME) ": [INFO] in %s(): ", __func__);\
	fprintf(stdout, __VA_ARGS__);}

#endif /* GLOBAL_H */
