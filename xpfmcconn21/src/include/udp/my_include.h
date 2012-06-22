#include <cstring>		// memset
#include <cstdio>		// printf, perror
#include <cstdlib>		// exit
#ifndef WIN_32
#include <unistd.h>		// sleep, select, close
#endif
#include <ctime>		// time related stuff

#define BUFF_LEN 50
