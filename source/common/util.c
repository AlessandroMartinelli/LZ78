#include "util.h"

void print_bytes(char *buf, int num){
	int i;

	printf("\t");

	for (i = 0; i < num; i++){
		printf("%02x ", (unsigned char)(buf[i]));
	}   
	printf("\n");
}

void csum(FILE *f, unsigned char *c){
	int i;
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (f == NULL) {
		// errno = ...
		LOG(ERROR,"Error in file opening");
		return;
	}

	MD5_Init(&mdContext);
	while((bytes = fread (data, 1, 1024, f)) != 0)
		MD5_Update(&mdContext, data, bytes);
	MD5_Final(c, &mdContext);
	//LOG(DEBUG, "File checksum:");
	//for(i = 0; i < MD5_DIGEST_LENGTH; i++) LOG(DEBUG,"%02x", c[i]);
	fseek(f, 0L, SEEK_SET); /* go back to the start of the file */
}
