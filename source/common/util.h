#include <sys/time.h>	//timeval
#include <stdio.h>		//fprintf, stderr
#include <stdint.h>		//for uintXX_t
#include <string.h>		//for str_cpy

#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_RESET	"\x1b[0m"
#define ANSI_COLOR_YELLOW	"\x1b[33m"
#define ANSI_COLOR_GREY		"\x1b[37m"

#define MAGIC 3

enum log_type {ERROR, WARNING, INFO, DEBUG};

extern uint8_t __verbose;


#define LOG(type, _fmt, ...)									\
do {															\
	char prefix[8];												\
	strcpy(prefix, "info");										\
	if(__verbose==1 || type != DEBUG){							\
	switch(type){												\
			case ERROR:											\
				fprintf(stdout, ANSI_COLOR_RED);				\
				strcpy(prefix, "error");						\
				break;											\
			case WARNING:										\
				fprintf(stdout, ANSI_COLOR_YELLOW);				\
				strcpy(prefix, "warning");						\
				break;											\
			case INFO:											\
				strcpy(prefix, "info");							\
				break;											\
			case DEBUG:											\
				fprintf(stdout, ANSI_COLOR_GREY);				\
				strcpy(prefix, "debug");						\
				break;											\
		}														\
		struct timeval _t0;										\
		gettimeofday(&_t0, NULL);								\
		fprintf(stdout, "[%s: %s, %d] %s: %03d.%06d   " _fmt "\n",	\
			__FILE__, __FUNCTION__, __LINE__, prefix,				\
			(int)(_t0.tv_sec % 1000), (int)_t0.tv_usec,				\
			##__VA_ARGS__);											\
		fprintf(stderr, ANSI_COLOR_RESET);							\
		}															\
}while(0)