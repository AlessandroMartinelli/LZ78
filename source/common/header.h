#ifndef _HEADER_H
#define _HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>		//for uintXX_t
#include <stdlib.h>
#include <openssl/md5.h>

struct header_t{
	uint64_t original_size;
	char *filename;	
	unsigned char *checksum;
	uint32_t magic_num;
	uint32_t dictionary_size;
	uint8_t symbol_size;

};
	
/* Writes the header on the file. 
 * If an error occurs, returns a negative value 
 */
int header_write(struct header_t *h, FILE *f);

/* Reads the file and writes the read values on the header_t structure
 */
int header_read(struct header_t *h, FILE *f);

/* This function must be used only after header_read usages
 * It frees the memory allocated for containing filename and checksum
 */
void header_free(struct header_t *h);

#endif
