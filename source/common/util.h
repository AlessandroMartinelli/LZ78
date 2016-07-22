#ifndef _UTIL_H
#define _UTIL_H

#include <sys/time.h>		//timeval
#include <stdio.h>			//fprintf, stderr
#include <stdint.h>			//for uintXX_t
#include <string.h>			//for str_cpy
#include <openssl/md5.h>	//checksum

#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_RESET	"\x1b[0m"
#define ANSI_COLOR_YELLOW	"\x1b[33m"
#define ANSI_COLOR_GREY		"\x1b[37m"

#define MAGIC 3

enum log_type {ERROR, WARNING, INFO, DEBUG};

uint8_t __verbose; 

#define LOG(type, _fmt, ...)										\
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
			fprintf(stdout, "[%s: %s, %d] %s: " _fmt "\n",			\
				__FILE__, __FUNCTION__, __LINE__, prefix,				\
				##__VA_ARGS__);											\
			fprintf(stderr, ANSI_COLOR_RESET);							\
			}															\
	}while(0)
	
void csum(FILE *f, unsigned char *c);

#endif

