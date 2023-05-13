#include <config.h>

#include <string.h>

#include "global.h"


extern char **environ;

int env_debug()
{
	int i;
	for (i = 0; environ[i] != NULL; i++) {
		if (strcmp(environ[i], "DEBUG") == '=')
			return 1;
	}
	return 0;
}
