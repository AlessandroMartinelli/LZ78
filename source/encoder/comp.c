#include "comp.h"

void csum(FILE *f, unsigned char *c){
	int i;
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];

	if (f == NULL) {
		LOG(ERROR,"Error in file opening");
		return -1;
	}

	MD5_Init(&mdContext);
	while((bytes = fread (data, 1, 1024, f)) != 0)
		MD5_Update(&mdContext, data, bytes);
	MD5_Final(c, &mdContext);
	LOG(DEBUG, "File checksum:");
	for(i = 0; i < MD5_DIGEST_LENGTH; i++) LOG(DEBUG,"%02x", c[i]);
	fseek(f, 0L, SEEK_SET); /* go back to the start of the file */
}

int comp(const char *filename_in, const char *filename_out, uint32_t dictionary_size, uint8_t symbol_size){
	struct bitio *b_out, *b_in;
	uint32_t aux_32;
	uint64_t aux_64;
	char aux_char;
	list_t *node;
	int res;
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
		return -1;
	}
	
	//( ---create the header of b_out
	FILE *f = bitio_get_file(b_out);
	struct stat *buf;
	fstat(f,buf);			/* compute statistics (original_size) */
	csum(f, checksum);	/* compute checksum */
	header_t h = { buf->st_size,		/* original_size */
						filename_in, 		/* filename */
						checksum,			/* checksum */
						MAGIC,				/* magic number */
						dictionary_size,	/* dictionary_size */
						symbol_size};		/* symbol_size */
	if(!header_write(h, t)){
		LOG(ERROR, "Header write gone wrong");
		return -1;
	}
	//---)
	
	/* declaration of the hash table (tree abstractoion)*/
	hash_table_t *t;
	t = create_hash_table(dictionary_size); //??
	
	while(sizeof(char)*8 == (res = bitio_read(b_in, sizeof(char)*8, &aux_64))){
	/* untill characters are available */
		aux_char = (char) aux_64;
		LOG(DEBUG,"read %d bits (char=%c)\",res, aux_char);
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
	
	free_table(t);
	
	bitio_close(b_out);
	bitio_close(b_in);
	return 0;
}
