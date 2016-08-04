/* decomp.c
 * 
 * Description----------------------------------------------------------
 * Definition of the behaviors of the compressor.
 * Implementation of the methods declared in "outoder_dictionary.h".
 * ---------------------------------------------------------------------
 */

#include "decomp.h"

int decode(code_t *array, code_t node, struct bitio* b, uint8_t symbol_size){
	int ret = 0;
	//LOG(DEBUG, "\tDecode <\"%c\", %lu>", node.character, node.parent_id);
	if(node.parent_id!=0){
		decode(array, array[node.parent_id - 1], b, symbol_size);
	}
	//LOG(DEBUG, "emit <\"%c\", %lu>", node.character, node.parent_id);
	ret = bitio_write(b, symbol_size, node.character); /* emit character */
	if (ret < 0){
		return -1;
	}
	
	return 0;
}

int decomp(const struct gstate *state, const uint64_t f_dim){
	int ret_symbol;
	int ret_id;
	int ret = 0;
	char aux_char;
	uint32_t size; //TODO: maybe a more explanatory name?
	uint32_t dictionary_len;
	uint64_t num_of_codes;
	uint64_t aux_64;
	uint8_t symbol_size;
	uint16_t id_size;
	uint64_t i;
	FILE* input_file_ptr = NULL;

	if(state->header->magic_num != MAGIC){
		LOG(ERROR,"Wrong decompression/wrong file");
		return -1;
	}
	
	dictionary_len = state->header->dictionary_len;
	id_size = ceil_log2(dictionary_len); /* log base 2 */
	symbol_size = state->header->symbol_size;
	LOG(DEBUG, "Check values read:\n\tdictionary_len: %u\n\tid_size: %d\n\tsymbol_size: %d", dictionary_len, id_size, symbol_size);
	//---)
	
	/* TODO: here we could put the calculation of f_dim, as of now
	 * performed in decomp_init_gstate().
	 */
	 
	/* get the number of codes */	 
	input_file_ptr = bitio_get_file(state->b_in);	
	num_of_codes = f_dim - ftell(input_file_ptr);
	num_of_codes *= 8; // bytes to bits
	num_of_codes /= symbol_size+id_size;	/* normalize the number of array
														 * elements by the code dimension*/
	LOG(INFO,"num_of_codes: %lu", num_of_codes);
	
	/* the dictionary is cleaned every dictionary_len codes */
	size = (num_of_codes <= dictionary_len)? num_of_codes : dictionary_len;
	LOG(INFO,"Dictionary size: %d", size); 
	code_t nodes[size];
	
	for(i=0; i<num_of_codes; i++){
		/* clean of the dictionary not needed */
		if(i!=0 && i%dictionary_len == 0) LOG(WARNING, "Clean the dictionary");
		
		ret_symbol = bitio_read(state->b_in, symbol_size, &aux_64);
		if (ret_symbol < 0){
			LOG(ERROR, "Read failed: %s", strerror(errno));
			return -1;
		}
		
		aux_char = (char) aux_64;
		ret_id = bitio_read(state->b_in, id_size, &aux_64);
		if (ret_id < 0){
			LOG(ERROR, "Read failed: %s", strerror(errno));
			return -1;
		}		
		
		if(ret_symbol == symbol_size && ret_id == id_size){
			nodes[i%dictionary_len].character = (char) aux_char;
			nodes[i%dictionary_len].parent_id = aux_64;
			LOG(DEBUG, "Code read #%lu: <\"%c\", %lu>", i%dictionary_len,
				nodes[i%dictionary_len].character,
				nodes[i%dictionary_len].parent_id);
			ret = decode(nodes, nodes[i%dictionary_len], state->b_out, symbol_size);
			if (ret < 0){
				LOG(ERROR, "Decode failed: %s", strerror(errno));
				return -1;
			}				
		}
		else{
			nodes[i%dictionary_len].character = (char) aux_char;
			nodes[i%dictionary_len].parent_id = aux_64;
			LOG(ERROR,"Code unreadable <\"%c\", %lu>",
				nodes[i%dictionary_len].character,
				nodes[i%dictionary_len].parent_id);
			ret = decode(nodes, nodes[i%dictionary_len], state->b_out, symbol_size);
			if (ret < 0){
				LOG(ERROR, "Decode failed: %s", strerror(errno));
			}
			return -1;
		}
	}

	/* flush bitio buffer */
	/*ret = bitio_close(state->b_out);
	state->b_out = NULL;
	if (ret < 0){
			LOG(ERROR, "Close failed: %s", strerror(errno));
			return -1;
	}*/
	
	return 0;
}

int decomp_check(const struct gstate *state){
	int ret;
	struct stat stat_buf;	
	unsigned char checksum[MD5_DIGEST_LENGTH];	
	
	ret = stat(state->header->filename, &stat_buf);
	if (ret == -1){
		LOG(ERROR, "Impossibile to create header_t structure: %s", strerror(errno));
		return -1;		
	}
	
	if((uint64_t)stat_buf.st_size != state->header->original_size){
		LOG(ERROR,"Original size error");
		return -1;
	}
	LOG(INFO, "Size match: OK!");
	
	/* compare checksum */
	csum(state->header->filename, checksum);
	if (checksum == NULL){
		LOG(ERROR, "Checksum calculation failed: %s", strerror(errno));
		return -1;
	}		
	LOG_BYTES(DEBUG, state->header->checksum, MD5_DIGEST_LENGTH, "Received checksum: ");
	LOG_BYTES(DEBUG, checksum, MD5_DIGEST_LENGTH, "Calculated checksum: ");			
	
	if(memcmp((char*)checksum, (char*)state->header->checksum,MD5_DIGEST_LENGTH) != 0){
		LOG(ERROR, "Checksum error: received and calculated checksum don't match");
		return -1;		
	} 
	LOG(INFO, "Checksum match: OK!");

	return 0;
}

int fake_decomp(const struct gstate *state){
	FILE* f_in = bitio_get_file(state->b_in);
	FILE* f_out = bitio_get_file(state->b_out);
	char buff[1024];
	unsigned int ret = 0;
	while((ret=fread(buff, 1, 1024, f_in))>0){
		if((fwrite(buff, 1, ret, f_out))!=ret){ 
			errno = ENOSPC;
			LOG(ERROR, "Impossible to write output file: %s", strerror(errno));
			return -1;
		}
	}

	return 0;
}
