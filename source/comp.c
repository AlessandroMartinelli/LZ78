#include "comp.h"

int comp(const struct gstate *state){
	hash_table_t *h_table = NULL;
	list_t *node = NULL;	
	uint64_t aux_64;
	char aux_char;
	int ret;
	uint16_t next_id;
	uint16_t parent_id;
	uint8_t id_size;
	uint32_t dictionary_len;
	uint8_t symbol_size;
		
	/* initialization of the hash table (tree abstraction) 
	 * and other variables 
	 */
	dictionary_len = state->header->dictionary_len;	 
	h_table = create_hash_table(dictionary_len/AVG_CODES_PER_ENTRY); //??
	if(h_table == NULL){
		LOG(ERROR, "Dictionary allocation failed: %s%s", strerror(errno),
			(errno == ENOMEM) ? ". Try with a smaller dictionary length":"");
		ret = -1;
		goto end;
	}
	parent_id = 0;
	next_id = 1;
	id_size = ceil_log2(dictionary_len); /* log base 2 */;	
	symbol_size = state->header->symbol_size;
	
	LOG(DEBUG, "Check values read:\n\tdictionary_size: %u\n\tid_size: " 
		"%d\n\tsymbol_size: %d", dictionary_len, id_size, symbol_size);
	
	while(sizeof(char)*8 == (ret = bitio_read(state->b_in, sizeof(char)*8, &aux_64))){
	/* as long as characters are available */
		aux_char = (char) aux_64;
		//LOG(DEBUG, "read %d bits (char=%c)", ret, aux_char);
		node = lookup_code(h_table, aux_char, parent_id);
		if(node == NULL){ 
			/* node not found */
			add_code(h_table, aux_char, parent_id, next_id++);
			LOG(DEBUG,"emit code #%d: <\"%c\", %d>", next_id-1, aux_char, parent_id);
			
			ret = bitio_write(state->b_out, symbol_size, (uint64_t) aux_char); /* emit character */
			if (ret < 0){
				LOG(ERROR, "Write failed: %s", strerror(errno));
				ret = -1;
				goto end;
			}
			
			ret = bitio_write(state->b_out, id_size, parent_id); /* emit parent_id */
			if (ret < 0){
				LOG(ERROR, "Write failed: %s", strerror(errno));
				ret = -1;
				goto end;
			}			
			parent_id = 0;
			
			/* dictionary is full: clean */
			if(next_id > dictionary_len){
				LOG(DEBUG, "Dictionary full!");
				next_id = 1;
				free_table(h_table);
				h_table = create_hash_table(dictionary_len/AVG_CODES_PER_ENTRY);
			}
		}
		else{
			//LOG(DEBUG,"code found \"%c\"", aux_char);
			parent_id = node->child_id;
		}
	}
	/* just to be sure it will emits all the characters? */
	if(node != NULL){
		LOG(DEBUG,"<\"%c\", %d>", node->character, node->parent_id);
		ret = bitio_write(state->b_out, symbol_size, (uint64_t) node->character); /* emit character */
		if (ret < 0){
			LOG(ERROR, "Write failed: %s", strerror(errno));
			ret = -1;
			goto end;
		}
						
		ret = bitio_write(state->b_out, id_size, node->parent_id); /* emit parent_id */
		if (ret < 0){
			LOG(ERROR, "Write failed: %s", strerror(errno));
			ret = -1;
			goto end;
		}			
	}
	LOG(INFO,"Compression terminated");
	ret = 0;
	
end:
	if(h_table!=NULL) free_table(h_table);
	return ret;
}


int fake_comp(const struct gstate* state){ /* TODO: write the header and copy the file */
	FILE* f_in = bitio_get_file(state->b_in);
	FILE* f_out = bitio_get_file(state->b_out);
	char buff[1024];
	int ret = 0;
	while((ret=fread(buff, 1024, 1, f_in))>0){
		if((fwrite(buff, ret, 1, f_out))!=1){ 
			errno = ENOSPC;
			if(state->b_in != NULL) bitio_close(state->b_in);
			if(state->b_out != NULL) bitio_close(state->b_out);
			return -1;
		}
	}
	if(state->b_in != NULL) bitio_close(state->b_in);
	if(state->b_out != NULL) bitio_close(state->b_out);
	return 0;
	
	return 0;
}
