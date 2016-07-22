#include "comp.h"

#define ROLLBACK()									\
	if(t!=NULL) free_table(t);						\
	if(b_in != NULL) bitio_close(b_in);			\
	if(b_out != NULL) bitio_close(b_out);
	

int comp(char *filename_in, char *filename_out, uint32_t dictionary_size, uint8_t symbol_size){
	struct bitio *b_out, *b_in;
	uint64_t aux_64;
	char aux_char;
	list_t *node;
	int res;
	struct stat *buf = NULL;
	uint16_t next_id = 1;
	uint16_t parent_id = 0;
	uint16_t id_size = (uint16_t) ceil(log(dictionary_size)); /* log base 2 */
	unsigned char checksum[MD5_DIGEST_LENGTH];
	
	/* files opening */
	b_out = bitio_open(filename_out, WRITE);
	if(filename_out == NULL) filename_in = strcat(filename_out,".lz78");
	b_in = bitio_open(filename_in, READ);
	if(b_out == NULL || b_in == NULL){
		LOG(ERROR, "Invalid file");
		ROLLBACK();
		return -1;
	}
	
	//( ---create the header of b_out
	FILE *f = bitio_get_file(b_out);
	fstat(fileno(f), buf);			/* compute statistics (original_size) */
	csum(f, checksum);	/* compute checksum */
	struct header_t h = { buf->st_size,		/* original_size */
						filename_in, 		/* filename */
						checksum,			/* checksum */
						(uint32_t) MAGIC,				/* magic number */
						dictionary_size,	/* dictionary_size */
						symbol_size};		/* symbol_size */
	if(!header_write(&h, f)){
		LOG(ERROR, "Header write gone wrong");
		ROLLBACK();
		return -1;
	}
	//---)
	
	/* declaration of the hash table (tree abstractoion)*/
	hash_table_t *t;
	t = create_hash_table(dictionary_size); //??
	
	while(sizeof(char)*8 == (res = bitio_read(b_in, sizeof(char)*8, &aux_64))){
	/* untill characters are available */
		aux_char = (char) aux_64;
		LOG(DEBUG, "read %d bits (char=%c)",res, aux_char);
		node = lookup_code(t, aux_char, parent_id);
		if(node == NULL){ /* node not found */
		
			add_code(t, aux_char, parent_id, next_id++);
			LOG(DEBUG,"emit code #%d: <\"%c\", %d>", next_id-1, aux_char, parent_id);
			
			bitio_write(b_out, symbol_size, (uint64_t) aux_char); /* emit character */
			bitio_write(b_out, id_size/*XXX*/, parent_id); /* emit parent_id */
			
			parent_id = 0;
			
			/* dictionary is full: clean */
			if(next_id == dictionary_size){
				LOG(WARNING, "Dictionary full!");
				next_id = 0;
				free_table(t);
				t = create_hash_table(dictionary_size);
			}
		}
		else{
			LOG(DEBUG,"code found \"%c\"", aux_char);
			parent_id = node->child_id;
		}
	}
	/* just to be sure it will emits all the characters? */
	if(node != NULL){
		LOG(DEBUG,"<\"%c\", %d>", node->character, node->parent_id);
		bitio_write(b_out, symbol_size, (uint64_t) aux_char); /* emit character */
		bitio_write(b_out, id_size/*XXX*/, parent_id); /* emit parent_id */
	}
	LOG(INFO,"Compression terminated");
	
	ROLLBACK();
	return 0;
}
