/* outomp.c
 * 
 * Description----------------------------------------------------------
 * Definition of the behaviors of the outompressor.
 * Implementation of the methods outlared in "outoder_dictionary.h".
 * ---------------------------------------------------------------------
 */

#include "decomp.h"

#define ROLLBACK()									\
	if(h != NULL) header_free(h);					\
	if(b_in != NULL) bitio_close(b_in);			\
	if(b_out != NULL) bitio_close(b_out);


void decode(code_t *array, code_t node, struct bitio* b, uint8_t symbol_size){
	if(node.parent_id!=0){
		decode(array, array[node.parent_id - 1], b, symbol_size);
	}
	bitio_write(b, symbol_size, node.character); /* emit character */
	return;
}

int decomp(const char *filename_in, const char *filename_out){
	struct bitio *b_in, *b_out;
	char aux_char;
	uint32_t size;
	uint64_t num_of_codes, f_curr;
	uint64_t aux_64;
	uint32_t dictionary_size;
	uint8_t symbol_size;
	uint16_t id_size;
	unsigned char checksum[MD5_DIGEST_LENGTH];
	uint8_t checksum_match = 1;
	struct header_t *h = NULL;
	struct stat *buf = NULL;
	
	if(filename_in == NULL){
		LOG(ERROR,"Input filename missing");
		ROLLBACK();
		return -1;
	}
	b_in = bitio_open(filename_in, READ);
	if(b_in == NULL){
		LOG(ERROR, "Input file impossible to be open");
		ROLLBACK();
		return -1;
	}
	
	//( ---read the header of b_in
	FILE *f = bitio_get_file(b_in);
	if(header_read(h, f) != 0){
		LOG(ERROR, "Header read gone wrong");
		ROLLBACK();
		return -1;
	}
	if(h->magic_num != MAGIC){
		LOG(ERROR,"Wrong decompression/wrong file");
		ROLLBACK();
		return -1;
	}
	dictionary_size = h->dictionary_size;
	id_size = (uint16_t) ceil(log(dictionary_size)); /* log base 2 */
	symbol_size = h->symbol_size;
	//---)
	
	b_out = bitio_open(filename_out==NULL ? h->filename : filename_out, WRITE);
	if(b_out == NULL){
		LOG(ERROR, "Output file impossible to be open");
		ROLLBACK();
		return -1;
	}
	
	/* get the number of codes */
	f_curr = ftell(f);
	fseek(f, 0L, SEEK_END);
	num_of_codes = ftell(f) - f_curr;
	fseek(f, 0L, f_curr);
	num_of_codes /= symbol_size+id_size;     /* normalize the number of array
													      * elements by the code dimension*/
	/* the dictionary is clean every dictionary_size codes */
	size = (num_of_codes <= dictionary_size)? num_of_codes : dictionary_size;
	LOG(INFO,"size: %d\n",size);
	
	code_t nodes[size];
	
	for(uint64_t i=0; i<num_of_codes; i++){
		if(bitio_read(b_in, id_size, &aux_64) == id_size &&
			bitio_read(b_in, symbol_size, (uint64_t*) &aux_char) == symbol_size){
			nodes[i%size].character = (char) aux_char;
			nodes[i%size].parent_id = aux_64;
			LOG(INFO, "<\"%c\", %lu>\n", nodes[i%size].character, nodes[i%size].parent_id);
			decode(nodes, nodes[i%size], b_out, symbol_size);
		}
		else{
			LOG(ERROR,"Code unreadable");
		ROLLBACK();
		return -1;
		}
		/*clean of the dictionary not needed*/
	}
	
	/* compare headers */
	FILE* f_out = bitio_get_file(b_out);
	csum(f_out, checksum);
	for(int i=0; i<MD5_DIGEST_LENGTH; i++){
		if(checksum[i] == h->checksum[i]) continue;
		LOG(WARNING, "Checksum error");
		checksum_match = 0;
		break;
	}
	if(checksum_match){
		fstat(fileno(f_out),buf);
		if(buf->st_size != h->original_size)
			LOG(WARNING,"Original size error");
	}
	ROLLBACK();
	return 0;
}
