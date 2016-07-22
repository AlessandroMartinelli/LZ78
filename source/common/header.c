#include "header.h"

int header_write(struct header_t *h, FILE *f){
	int ret;
	ret = fprintf(f,"%u%u%uhh%d%s%ld",
		h->magic_num, h->dictionary_size, h->symbol_size, strlen(h->filename),
		h->filename, h->original_size);

	if(ret<0){
		return ret;
	}

	ret = fwrite(h->checksum, MD5_DIGEST_LENGTH,1,f);
	return ret;
}

int header_read(struct header_t *h, FILE *f){	
	int filename_size; 
	int ret;
	char *buf;
	ret = fscanf(f,"%u%u%hhu%d",&(h->magic_num), &(h->dictionary_size), 
		&(h->symbol_size), &filename_size);
	if(ret<=0){
		return ret;
	}
	
	//allocates a single block of memory shared among filename and checksum
	buf = (char*)malloc(MD5_DIGEST_LENGTH + filename_size);
	h->filename = buf;
	h->checksum = (unsigned char*)(buf + filename_size);
	ret = fscanf(f,"%s%ld",		
		h->filename, &(h->original_size));
	if(ret<=0){
		return ret;
	}

	ret = fread(h->checksum, MD5_DIGEST_LENGTH, 1, f);
	return ret;
}

void header_free(struct header_t *h){
	//frees also the checksum because is a contiguous block of memory
	free(h->filename);
}