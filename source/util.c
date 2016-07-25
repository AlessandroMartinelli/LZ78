#include "util.h"

void csum(const char *filename, unsigned char *c){
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	FILE* fd = fopen(filename, "r");

	if (fd == NULL) {
		errno = ENOENT;
		c = NULL;
		return;
	}

	MD5_Init(&mdContext);
	while((bytes = fread (data, 1, 1024, fd)) != 0)
		MD5_Update(&mdContext, data, bytes);
	MD5_Final(c, &mdContext);
	//LOG(DEBUG, "File checksum:");
	//for(i = 0; i < MD5_DIGEST_LENGTH; i++) LOG(DEBUG,"%02x", c[i]);
	fclose(fd);
}

uint8_t ceil_log2(uint32_t x){
	uint32_t w = x-1;
	uint8_t y;
	if(x<1) return 0;

	//uint8_t y = (((x & (x - 1)) == 0) ? 0 : 1); /* power of 2 */
	for(y=0; w>>y != 0; y++);
	
	return y;
}
