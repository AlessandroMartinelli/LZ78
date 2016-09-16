#include "comp.h"

/* pre-processing:
 * initialize the tree (hash table) with all the possible simbols
 */
int comp_preprocessing(hash_table_t *h, uint8_t symbol_size){
	int i;
	for(i=0; i<(1<<symbol_size); i++){
		//add all possible character as root(0) children
		if (add_code(h, (char) i, 0, i+1) != 0){
			LOG(ERROR, "No enough memory to add a new code: %s", strerror(errno));
			return -1;
		}
	}
	return 0;
}

int comp(const struct gstate *state){
	hash_table_t *h_table = NULL;
	list_t *node = NULL;	
	uint64_t aux_64;
	char aux_char;
	int ret;
	uint32_t next_id;
	uint32_t parent_id;
	uint8_t id_size;
	uint32_t dictionary_len;
	uint8_t symbol_size;
	
	/* initialization of the hash table (tree abstraction) and other variables */
	dictionary_len = state->header->dictionary_len;	 
	h_table = create_hash_table(dictionary_len/AVG_CODES_PER_ENTRY);//see compr.h
	if(h_table == NULL){
		LOG(ERROR, "Dictionary allocation failed: %s%s", strerror(errno),
			(errno == ENOMEM) ? ". Try with a smaller dictionary length":"");
		ret = -1;
		goto end;
	}
	parent_id = 0;
	id_size = ceil_log2(dictionary_len); // log base 2
	symbol_size = state->header->symbol_size;
	next_id = (1<<symbol_size) + 1;
	
	LOG(DEBUG, "Check values read:\n\tdictionary_size: %" PRIu32 "\n\tid_size: " 
		"%" PRIu8 "\n\tsymbol_size: %" PRIu8, dictionary_len, id_size, symbol_size);
	
	if(comp_preprocessing(h_table, symbol_size) == -1){
		ret = -1;
		goto end;
	}
	
	/* read simbols until the EOF is reached, as long as characters are available */
	while(sizeof(char)*8 == (ret = bitio_read(state->b_in, sizeof(char)*8, &aux_64))){
		aux_char = (char) aux_64;
		node = lookup_code(h_table, aux_char, parent_id);
		
		/* node not found (add to the tree) */
		if(node == NULL){
			//LOG(DEBUG,"emit code #%" PRIu32 ", char \"%c\": %" PRIu32, next_id, aux_char, parent_id);
			ret = add_code(h_table, aux_char, parent_id, next_id++);
			if(ret != 0){
				LOG(ERROR, "No enough memory to add a new code: %s", strerror(errno));
				ret = -1;
				goto end;
			}
			
			/* emit character */
			// no need
		
			/* emit code */
			ret = bitio_write(state->b_out, id_size, parent_id);
			if (ret < 0){
				LOG(ERROR, "Write failed: %s", strerror(errno));
				ret = -1;
				goto end;
			}
			
			/* continue from the same character child of the root node */
			node = lookup_code(h_table, aux_char, 0);
			parent_id = node->child_id;
			
			/* dictionary is full: clean */
			if(next_id > dictionary_len){
				LOG(DEBUG, "Dictionary is full. Reconstructing from scratch...");
				free_table(h_table);
				h_table = create_hash_table(dictionary_len/AVG_CODES_PER_ENTRY);
				if(h_table == NULL){
					LOG(ERROR, "Dictionary allocation failed: %s%s", strerror(errno),
						(errno == ENOMEM) ? ". Try with a smaller dictionary length":"");
					ret = -1;
					goto end;
				}
				if(comp_preprocessing(h_table, symbol_size) == 0){
					ret = -1;
					goto end;
				}
				next_id = (1<<symbol_size) + 1; // shift preprocessed characters
			}
		}
		/* move down throught the existing node of the tree */
		else{
			parent_id = node->child_id;
		}
	}
	/* last sequence was found but it was not emitted */
	if(node != NULL){
		//LOG(DEBUG,"emit code \"last\", char \"%c\": %" PRIu32, node->character, node->child_id);
		
		/* emit character */
		// no need
		
		/* emit code */
		ret = bitio_write(state->b_out, id_size, node->child_id); 
		if (ret < 0){
			LOG(ERROR, "Write failed: %s", strerror(errno));
			ret = -1;
			goto end;
		}
	}
	
	/* emit EOF simbol (AKA root code) */
	ret = bitio_write(state->b_out, id_size, 0); 
	if (ret < 0){
		LOG(ERROR, "Write failed: %s", strerror(errno));
		ret = -1;
		goto end;
	}
	
	
	LOG(INFO,"Compression terminated");
	ret = 0;
	
end:
	if(h_table!=NULL) free_table(h_table);
	return ret;
}


/* Write the header and copy the file */
int fake_comp(const struct gstate* state){
	FILE* f_in = fopen(state->input_file, "r");
	FILE* f_out = fopen(state->output_file, "w");
	if(f_in == NULL || f_out == NULL){
		LOG(ERROR, "Impossible to open files: %s", strerror(errno));
		return -1;
	}
	char buff[1024];
	unsigned int ret = 0;
	/* write header */
	if(header_write(state->header, f_out) != 0){
		LOG(ERROR, "Impossible to write the header (%s): %s", state->output_file, strerror(errno));
		return -1;
	}
	
	/* copy the file */
	while((ret=fread(buff, 1, 1024, f_in))>0){
		if((fwrite(buff, 1, ret, f_out))!=ret){ 
			errno = ENOSPC;
			LOG(ERROR, "Impossible to write output file (%s): %s", state->output_file, strerror(errno));
			return -1;
		}
	}
	fclose(f_in);
	fclose(f_out);
	
	return 0;
}
