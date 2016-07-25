#ifndef _UTIL_H
#define _UTIL_H

#include <sys/time.h>		//timeval
#include <stdio.h>			//fprintf, stderr
#include <stdint.h>			//for uintXX_t
#include <string.h>			//for str_cpy
#include <openssl/md5.h>	//checksum
#include <errno.h>			// for using the variabile errno
#include <stdlib.h>			// for calloc

#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_RESET	"\x1b[0m"
#define ANSI_COLOR_YELLOW	"\x1b[33m"
#define ANSI_COLOR_BLUE		"\x1b[34m"

#define MAGIC 3

enum log_type {ERROR, WARNING, INFO, DEBUG};

#define LOG(type, _fmt, ...)										\
	do {																	\
		char prefix[8];												\
		strcpy(prefix, "info");										\
		if(__verbose==1 || type != DEBUG){						\
		switch(type){													\
				case ERROR:												\
					fprintf(stdout, ANSI_COLOR_RED);				\
					strcpy(prefix, "ERROR");						\
					break;												\
				case WARNING:											\
					fprintf(stdout, ANSI_COLOR_YELLOW);			\
					strcpy(prefix, "WARNING");						\
					break;												\
				case INFO:												\
					strcpy(prefix, "INFO");							\
					break;												\
				case DEBUG:												\
					fprintf(stdout, ANSI_COLOR_BLUE);			\
					strcpy(prefix, "DEBUG");						\
					break;												\
			}																\
			fprintf(stdout, "[%s: %s, %d] %s: " _fmt "\n",	\
				__FILE__, __FUNCTION__, __LINE__, prefix,		\
				##__VA_ARGS__);										\
			fprintf(stderr, ANSI_COLOR_RESET);					\
			}																\
	}while(0)

uint8_t __verbose; 

void print_bytes(char *buf, int num);
	
/* CSUM
 *  Return an MD5-checksum calculated on the file pointed by the given
 *  file pointer.
 */ 
void csum(const char *filename, unsigned char* c);

uint8_t ceil_log2(uint32_t x);

#endif

