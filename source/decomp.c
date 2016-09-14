/* decomp.c
 * 
 * Description----------------------------------------------------------
 * Definition of the behaviors of the compressor.
 * Implementation of the methods declared in "outoder_dictionary.h".
 * ---------------------------------------------------------------------
 */

#include "decomp.h"

int decode(code_t *array, code_t *node, struct bitio* b, uint8_t symbol_size, int i){
	int ret = 0;
	
	/* upward path to the root */
	if(node->parent_id!=0){
		decode(array, &array[node->parent_id], b, symbol_size, i);
	}
	/* now, reached the root, i know new node character */
	else{
		if(i > 1<<symbol_size){ // first code doesn't set any node.character
			array[i-1].character = node->character;
			LOG(DEBUG, "Previous char(%d): %c", i-1, node->character);
		}
	}
	
	/* emit character */
	LOG(DEBUG,"Decoding node %d (parent_id %d, char %c)", i, node->parent_id, node->character);
	ret = bitio_write(b, symbol_size, node->character);
	if (ret < 0){
		return -1;
	}
	
	return 0;
}

void decomp_preprocessing(code_t *n, uint8_t symbol_size){
	int i;
	for(i=0; i < 1<<symbol_size; i++){ //dictionary_len MUST be greater than # of char
		n[i].character = (char) i;
		n[i].parent_id = 0;
	}
	
	return;
}

int decomp(const struct gstate *state){
	int i;
	int ret_id;
	int ret = 0;
	uint32_t dictionary_len;
	uint64_t aux_64;
	uint8_t symbol_size;
	uint16_t id_size;

	if(state->header->magic_num != MAGIC_NORMAL){
		LOG(ERROR,"Wrong decompression/wrong file");
		return -1;
	}
	
	dictionary_len = state->header->dictionary_len;
	id_size = ceil_log2(dictionary_len); /* log base 2 */
	symbol_size = state->header->symbol_size;
	LOG(DEBUG, "Check values read:\n\tdictionary_len: %u\n\tid_size:"
		"%d\n\tsymbol_size: %d", dictionary_len, id_size, symbol_size);
	
	code_t nodes[dictionary_len];
	decomp_preprocessing(nodes, symbol_size);
	
	for(i = 1<<symbol_size; 1; i++){ /* first 2^symb_size codes in array nodes are initialized */
		/* clean of the dictionary */
		if(i!=0 && i%dictionary_len == 0){
			LOG(WARNING, "Clean the dictionary");
			i = 1<<symbol_size; // shift the preprocessed characters
			/* no other actions are needed: overwrite the array */
		}
		
		/* read the symbol */
		// no need
		
		/* read the code */
		ret_id = bitio_read(state->b_in, id_size, &aux_64);
		if (ret_id < 0){
			LOG(ERROR, "Read failed: %s", strerror(errno));
			return -1;
		}
		
		/* the character will be set on the next scan */
		// nodes[i%dictionary_len].character =??
		nodes[i%dictionary_len].parent_id = aux_64;
		
		if(ret_id == id_size){
			if (aux_64 == 0){ /* read EOF, break infinite loop */
				break;
			}
			/* use ret_id as an index */
			ret_id = i%dictionary_len;
			
			/* i need start to decode from the parent of the new node,
			 * because i have no hint on the new node character yet.
			 * Problem: when to emit the new character?
			 * it's the first character of the next decode(...) */
			ret = decode(nodes, &nodes[aux_64], state->b_out, symbol_size, ret_id);
			
			if (ret < 0){
				LOG(ERROR, "Decode failed: %s", strerror(errno));
				return -1;
			}
		}
		else{
			/*LOG(ERROR,"Code unreadable <\"%c\", %llu>",
				nodes[i%dictionary_len].character,
				nodes[i%dictionary_len].parent_id);
			*/
			LOG(ERROR, "Decode failed: %s", strerror(errno));
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
	
	ret = stat(state->output_file, &stat_buf);
	if (ret == -1){
		LOG(ERROR, "Impossibile to compute statistics on file %s: %s",
			state->output_file, strerror(errno));
		return -1;		
	}
	
	if((uint64_t)stat_buf.st_size != state->header->original_size){
		LOG(ERROR,"Original size error");
		return -1;
	}
	LOG(INFO, "Size match: OK!");
	
	/* compare checksum */
	csum(state->output_file, checksum);
	if (checksum == NULL){
		LOG(ERROR, "Checksum calculation failed: %s", strerror(errno));
		return -1;
	}
	LOG_BYTES(DEBUG, state->header->checksum, MD5_DIGEST_LENGTH, "Received checksum: ");
	LOG_BYTES(DEBUG, checksum, MD5_DIGEST_LENGTH, "Computed checksum: ");			
	
	if(memcmp((char*)checksum, (char*)state->header->checksum,MD5_DIGEST_LENGTH) != 0){
		LOG(ERROR, "Checksum error: received and calculated checksum don't match");
		return -1;		
	} 
	LOG(INFO, "Checksum match: OK!");

	return 0;
}

/* remove header and copy */
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
