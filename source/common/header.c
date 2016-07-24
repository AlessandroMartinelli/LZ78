#include "header.h"

int header_write(struct header_t *h, FILE *f){
	int ret[7];
	uint32_t filename_len = strlen(h->filename)*sizeof(char);
	
	ret[0] = fwrite(&(h->magic_num), 4, 1, f);
	ret[1] = fwrite(&(h->dictionary_size), 4, 1, f);
	ret[2] = fwrite(&(h->symbol_size), 1, 1, f);
	ret[3] = fwrite(&filename_len, 4, 1, f);
	ret[4] = fwrite(h->filename, filename_len, 1, f);
	ret[5] = fwrite(&(h->original_size), 8, 1, f);
	ret[6] = fwrite(h->checksum, MD5_DIGEST_LENGTH, 1, f);
	
	for(int i=0; i<7; i++){
		if(ret[i]<0){
			//errno = ...
			return ret[i];
		} 
	}
	
	/*
	ret = fprintf(f,"%d%d%d%d%s%c%d",
		h->magic_num, h->dictionary_size, h->symbol_size, (int32_t)strlen(h->filename),
		h->filename, '\0', h->original_size);

	if(ret<0){
		return ret;
	}
	*/
	return 0;
}

int header_read(struct header_t *h, FILE *f){	
	char end_string;
	uint32_t filename_len; 
	int ret;
	char *buf = NULL;
	
	fread(&(h->magic_num), 4, 1, f);
	fread(&(h->dictionary_size), 4, 1, f);
	fread(&(h->symbol_size), 1, 1, f);
	fread(&filename_len, 4, 1, f);
	
	buf = (char*)malloc(filename_len + 1);	
	h->filename = buf;
	fread(h->filename, filename_len, 1, f);
	h->filename[filename_len] = '\0';	

	fread(&(h->original_size), 8, 1, f);
	
	h->checksum = (unsigned char*)malloc(MD5_DIGEST_LENGTH);
	ret = fread(h->checksum, MD5_DIGEST_LENGTH, 1, f);	
	printf("ret del checksum vale %d\n", ret);
	
	/*
	ret = fscanf(f,"%d%d%d%d",&(h->magic_num), &(h->dictionary_size), 
		&(h->symbol_size), &filename_size);
	if(ret<=0){
		//errno = ...
		return ret;
	}
	*/
	
	printf("magic_num: %d, dictionary_size: %d, symbol_size: %d, filename_len: %d\n",
		h->magic_num, h->dictionary_size, h->symbol_size, filename_len);
	/*
	//allocates a single block of memory shared among filename and checksum
	buf = (char*)calloc(1, MD5_DIGEST_LENGTH + filename_size + 1);
	
	h->filename = buf;
	ret = fscanf(f,"%s%c%d", h->filename, &end_string, &(h->original_size));
	if(ret<=0){
		return ret;
	}	
	
	h->checksum = (unsigned char*)(buf + filename_size + 1);
	ret += fread(h->checksum, MD5_DIGEST_LENGTH, 1, f);
	printf("after checksum fullfil, ret: %d\n", ret);
	
	printf("filename: %s, endstring: %c, original_size: %d, checksum: %s\n",
		h->filename, end_string, h->original_size, h->checksum);
	
	
	*/
	return ret;
}

void header_free(struct header_t *h){
	//frees also the checksum because is a contiguous block of memory
	free(h->filename);
}
