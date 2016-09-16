#include "header.h"

int header_write(struct header_t *h, FILE *f){
	int ret = 0;
	uint32_t magic_num_aux;
	uint32_t dictionary_len_aux;
	uint32_t filename_len_aux;
	uint64_t original_size_aux;
	uint32_t filename_len = strlen(h->filename);
	
#if __BYTE_ORDER == __BIG_ENDIAN
	LOG(DEBUG, "Big endian machine");
	
	magic_num_aux = htole32(h->magic_num);
	dictionary_len_aux = htole32(h->dictionary_len);
	filename_len_aux = htole32(filename_len);
	original_size_aux = htole64(h->original_size);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	LOG(DEBUG, "Little endian machine");
	
	magic_num_aux = h->magic_num;
	dictionary_len_aux = h->dictionary_len;
	filename_len_aux = filename_len;
	original_size_aux = h->original_size;
#else
	LOG(ERROR, "Byte order not recognized");
	return -1;
#endif
	
	ret += fwrite(&magic_num_aux, sizeof(uint32_t), 1, f);
	ret += fwrite(&dictionary_len_aux, sizeof(uint32_t), 1, f);
	ret += fwrite(&(h->symbol_size), sizeof(uint8_t), 1, f);
	ret += fwrite(&filename_len_aux, sizeof(uint32_t), 1, f);
	ret += fwrite(h->filename, filename_len + 1, 1, f);
	ret += fwrite(&original_size_aux, sizeof(uint64_t), 1, f);
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
	
#if __BYTE_ORDER == __BIG_ENDIAN	
	LOG(DEBUG, "Big endian machine");	
	h->magic_num = le32toh(h->magic_num);
	h->dictionary_len = le32toh(h->dictionary_len);	
	filename_len = le32toh(filename_len);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	LOG(DEBUG, "Little endian machine");		
	/* nothing to do. However, this check is necessary so that,
	 * if we enter the "else" branch, we know for sure
	 * our machine is neither big endian nor little endian
	 */	
#else
	LOG(ERROR, "Byte order not recognized");
	return -1;
#endif
		
	if (ret != 4){
		errno = ENODATA;
		return -1;
	}
	
	h->filename = malloc(filename_len + 1);
	if (h->filename == NULL){
		errno = ENOMEM;
		return -1;
	}
	ret += fread(h->filename, filename_len + 1, 1, f);
	
	ret += fread(&(h->original_size), sizeof(uint64_t), 1, f);
#if __BYTE_ORDER == __BIG_ENDIAN
	/* The check for "error in endianess" was previously performed,
	 * no need to repeat it 
	 */	
	h->original_size = le64toh(h->original_size);
#endif
	
	h->checksum = (unsigned char*)malloc(MD5_DIGEST_LENGTH);
	if (h->filename == NULL){
		errno = ENOMEM;		
		return -1;
	}	
	ret += fread(h->checksum, MD5_DIGEST_LENGTH, 1, f);	
	if (ret != 7){
		errno = ENODATA;
		return -1;	
	}
	
	return ret;
}

void header_free(struct header_t *h){ 
  if (h->filename) free(h->filename); 
  if (h->checksum) free(h->checksum); 
  h->filename = NULL;			
  h->checksum = NULL;
} 
