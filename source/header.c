#include "header.h"

int header_write(struct header_t *h, FILE *f){
	int ret = 0;
	uint32_t filename_len = strlen(h->filename)*sizeof(char);
	
	ret += fwrite(&(h->magic_num), sizeof(uint32_t), 1, f);
	ret += fwrite(&(h->dictionary_len), sizeof(uint32_t), 1, f);
	ret += fwrite(&(h->symbol_size), sizeof(uint8_t), 1, f);
	ret += fwrite(&filename_len, sizeof(uint32_t), 1, f);
	ret += fwrite(h->filename, filename_len + 1, 1, f);
	ret += fwrite(&(h->original_size), sizeof(uint64_t), 1, f);
	ret += fwrite(h->checksum, MD5_DIGEST_LENGTH, 1, f);
	
	if (ret != 7){
		errno = ENOSPC;
		return -1;
	}
	return 0;
}

int header_read(struct header_t *h, FILE *f){	
	uint32_t filename_len; 
	int ret = 0;
	
	ret += fread(&(h->magic_num), sizeof(uint32_t), 1, f);
	ret += fread(&(h->dictionary_len), sizeof(uint32_t), 1, f);
	ret += fread(&(h->symbol_size), sizeof(uint8_t), 1, f);
	ret += fread(&filename_len, sizeof(uint32_t), 1, f);
	
	if (ret != 4){
		errno = ENODATA;
		return -1;
	}
	
	h->filename = (char*)calloc(1, filename_len + 1);
	if (h->filename == NULL){
		errno = ENOMEM;
		return -1;
	}
	ret += fread(h->filename, filename_len + 1, 1, f);
	
	ret += fread(&(h->original_size), sizeof(uint64_t), 1, f);
	
	h->checksum = (unsigned char*)calloc(1, MD5_DIGEST_LENGTH);
	if (h->filename == NULL){
		errno = ENOMEM;
		header_read_free(h);		
		return -1;
	}	
	ret += fread(h->checksum, MD5_DIGEST_LENGTH, 1, f);	
	if (ret != 7){
		errno = ENODATA;
		header_read_free(h);
		return -1;	
	}
	
	LOG_BYTES(INFO, h->checksum, MD5_DIGEST_LENGTH, "The header structure has been read in the following way:\n"
		"\tOriginal size    = %ld\n"
		"\tOriginal filname = %s\n"
		"\tMAGIC number     = %d\n"
		"\tdictionary_len  = %u\n"
		"\tsymbol_size      = %u\n"
		"\tchecksum         = ",
		h->original_size, h->filename, h->magic_num, h->dictionary_len, h->symbol_size);
	
	return ret;
}

void header_read_free(struct header_t *h){
	if (h->filename) free(h->filename);
	if (h->checksum) free(h->checksum);
}

//h->filename[filename_len] = '\0';
