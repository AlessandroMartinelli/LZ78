#include "comp.h"

int comp(const char *filename_in, const char *filename_out, uint32_t dictionary_size){
	struct bitio *b_out, *b_in;
	uint16_t next_id = 1;
	uint16_t parent_id = 0;
	int res;
	char aux_char;
	uint32_t aux_32;
	uint64_t aux_64;
	list_t *node;
	
	/*
	if(check_ext(filename_out, "lz78") || check_ext(filename_in, "txt")){
		errno = EINVAL;
		return -1;
	}
	*/
	printf("comp1\n");
	
	b_out = bitio_open(filename_out, WRITE);
	b_in = bitio_open(filename_in, READ);
	if(b_out == NULL || b_in == NULL) return -1;
	
	//( ---create the header of b_out
	aux_32 = MAGIC;
	memcpy(&aux_64, &aux_32, sizeof(uint32_t));
	bitio_write(b_out, sizeof(uint32_t)*8, aux_64);
	
	memcpy(&aux_64, &dictionary_size, sizeof(uint32_t));
	bitio_write(b_out, sizeof(uint32_t)*8, aux_64);
	
	aux_32 = 8/*bits (character)*/ + 8/* XXX bits (parent_index)*/;
	memcpy(&aux_64, &aux_32, sizeof(uint64_t));
	bitio_write(b_out, sizeof(uint32_t)*8, aux_64);
	
	memcpy(&aux_64, filename_in, strlen(filename_in));
	bitio_write(b_out, sizeof(uint32_t)*8, aux_64);
	//---)
	
	/* declaration of the hash table (tree abstractoion)*/
	hash_table_t *t;
	t = create_hash_table(dictionary_size); //??
	
	LOG(INFO,"comp: read %d bits (char=%c)",res, aux_char);
	
	while(sizeof(char)*8 == (res = bitio_read(b_in, sizeof(char)*8, &aux_64))){
	/* untill characters are available*/
		aux_char = (char) aux_64;
		LOG(INFO,"read %d bits (char=%c)\",res, aux_char);
		node = lookup_code(t, aux_char, parent_id);
		if(node == NULL){ /* node not found */
		
			add_code(t, aux_char, parent_id, next_id++);
			LOG(DEBUG,"emit code #%d: <\"%c\", %d>", next_id-1, aux_char, parent_id);
			
			bzero(&aux_64, sizeof(uint64_t));
			memcpy(&aux_64, &aux_c, sizeof(char));
			bitio_write(b_out, sizeof(char)*8, aux_64); /* emit character */
			bitio_write(b_out, 8/*XXX*/, parent_id); /* emit parent_id */
			
			parent_id = 0;
			
			/* dictionary is full: clean */
			if(next_id == dictionary_size){ 
				next_id = 0;
				free_table(t);
				t = create_hash_table(dictionary_size);
			}
		}
		else{
			LOG(INFO,"code found \"%c\"", aux_char);
			parent_id = node->child_id;
		}
	}
	/* just to be sure it will emits all the characters? */
	if(node != NULL){
		LOG(INFO,"<\"%c\", %d>", node->character, node->parent_id);
		bzero(&aux_64, sizeof(uint64_t));
		memcpy(&aux_64, &aux_c, sizeof(char));
		bitio_write(b_out, sizeof(char)*8, aux_64); /* emit character */
		bitio_write(b_out, 8/*XXX*/, parent_id); /* emit parent_id */
	}
	
	free_table(t);
	
	bitio_close(b_out);
	bitio_close(b_in);
	return 0;
}
