#ifndef _UTIL_H
#define _UTIL_H

#include <sys/time.h>		//timeval
#include <stdio.h>			//fprintf, stderr
#include <stdint.h>			//for uintXX_t
#include <string.h>			//for str_cpy
#include <openssl/md5.h>	//checksum
#include <errno.h>			// for using the variabile errno
#include <stdlib.h>			// for calloc
#include "bitio.h"
#include "header.h"

#define ANSI_COLOR_RED		"\x1b[31m"
#define ANSI_COLOR_RESET	"\x1b[0m"
#define ANSI_COLOR_YELLOW	"\x1b[33m"
#define ANSI_COLOR_MAGENTA	"\x1b[35m"

#define MAGIC_NORMAL 3
#define MAGIC_FAKE 2

#define LOG(type, _fmt, ...)										\
	do {																	\
		char prefix[8];												\
		strcpy(prefix, "info");										\
		if(__verbose==1 || type != DEBUG){						\
		switch(type){													\
				case ERROR:												\
					printf(ANSI_COLOR_RED);				\
					strcpy(prefix, "ERROR");						\
					break;												\
				case WARNING:											\
					printf(ANSI_COLOR_YELLOW);			\
					strcpy(prefix, "WARNING");						\
					break;												\
				case INFO:												\
					strcpy(prefix, "INFO");							\
					break;												\
				case DEBUG:												\
					printf(ANSI_COLOR_MAGENTA);			\
					strcpy(prefix, "DEBUG");						\
					break;												\
			}																\
			printf("[%s: %s, %d] %s: " _fmt "\n",	\
				__FILE__, __FUNCTION__, __LINE__, prefix,		\
				##__VA_ARGS__);										\
			printf(ANSI_COLOR_RESET);					\
			}																\
	}while(0)

#define LOG_BYTES(type, buf, num, _fmt, ...)										\
	do {																\
		int i;															\
		char prefix[8];												\
		strcpy(prefix, "info");										\
		if(__verbose==1 || type != DEBUG){						\
		switch(type){													\
				case ERROR:												\
					printf(ANSI_COLOR_RED);				\
					strcpy(prefix, "ERROR");						\
					break;												\
				case WARNING:											\
					printf(ANSI_COLOR_YELLOW);			\
					strcpy(prefix, "WARNING");						\
					break;												\
				case INFO:												\
					strcpy(prefix, "INFO");							\
					break;												\
				case DEBUG:												\
					printf(ANSI_COLOR_MAGENTA);			\
					strcpy(prefix, "DEBUG");						\
					break;												\
			}																\
			printf("[%s: %s, %d] %s: " _fmt,	\
				__FILE__, __FUNCTION__, __LINE__, prefix,		\
				##__VA_ARGS__);											\
			for (i = 0; i < num; i++){									\
				fprintf(stdout, "%02x ", (unsigned char)(buf[i]));		\
			}														\
			printf(ANSI_COLOR_RESET);					\
			printf("\n");									\
			}																\
	}while(0)


enum log_type {ERROR, WARNING, INFO, DEBUG};

struct gstate{
	char* input_file;
	char* output_file;
	struct bitio* b_in;
	struct bitio* b_out;
	struct header_t* header;
};

uint8_t __verbose;
	
/* CSUM
 *  Return an MD5-checksum calculated on the file pointed by the given
 *  file pointer.
 */ 
void csum(const char *filename, unsigned char* c);

uint8_t ceil_log2(uint32_t x);

#endif

