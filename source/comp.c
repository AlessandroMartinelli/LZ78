#include "comp.h"

int comp(char *input_file, char *output_file, uint32_t dictionary_size, uint8_t symbol_size){
	struct bitio *b_out = NULL;
	struct bitio *b_in = NULL;
	struct stat *stat_buf = NULL;	
	struct header_t header;
	hash_table_t *h_table = NULL;
	list_t *node = NULL;	
	uint64_t aux_64;
	char aux_char;
	int ret;
	uint16_t next_id;
	uint16_t parent_id;
	uint8_t id_size = ceil_log2(dictionary_size); /* log base 2 */;
	unsigned char checksum[MD5_DIGEST_LENGTH];
	
	
	LOG(DEBUG, "Check values read:\n\tdictionary_size: %u\n\tid_size: " 
		"%d\n\tsymbol_size: %d", dictionary_size, id_size, symbol_size);
	
	/* Allocation and initialization of stat structure */
	stat_buf = calloc(1, sizeof(struct stat));
	if (stat_buf == NULL){
		errno = ENOMEM;
		LOG(ERROR, "Impossibile to create stat struct: %s", strerror(errno));	
		ret = -1;
		goto end;		
	}
	stat(input_file, stat_buf);	
	
	
	
	/* Compute checksum of input file */
	csum(input_file, checksum);
	if (checksum == NULL){
		LOG(ERROR, "Impossibile to calculate csum: %s", strerror(errno));
		ret = -1;
		goto end;
	}
	
	/* Filling of header_t structure */
	header = (struct header_t){ 
		stat_buf->st_size,			/* original_size */
		basename(input_file), 		/* input file name */
		checksum,					/* checksum */
		MAGIC,						/* magic number */
		dictionary_size,			/* dictionary_size */
		symbol_size					/* symbol_size */
	};
	
	LOG_BYTES(INFO, header.checksum, MD5_DIGEST_LENGTH, 
		"The header structure has been filled in the following way:\n"
		"\tOriginal size    = %ld\n"
		"\tOriginal filname = %s\n"
		"\tMAGIC number     = %d\n"
		"\tdictionary_size  = %u\n"
		"\tsymbol_size      = %u\n"
		"\tchecksum         = ",
		header.original_size, header.filename,
		header.magic_num, header.dictionary_size, header.symbol_size);
	

		
	/* initialization of the hash table (tree abstraction) 
	 * and other variables 
	 */
	h_table = create_hash_table(dictionary_size/AVG_CODES_PER_ENTRY); //??
	if(h_table == NULL){
		LOG(ERROR, "Dictionary allocation failed: %s%s", strerror(errno),
			(errno == ENOMEM) ? ". Try with a smaller dictionary length":"");
		ret = -1;
		goto end;
	}
	parent_id = 0;
	next_id = 1;
	
	while(sizeof(char)*8 == (ret = bitio_read(b_in, sizeof(char)*8, &aux_64))){
	/* as long as characters are available */
		aux_char = (char) aux_64;
		//LOG(DEBUG, "read %d bits (char=%c)", ret, aux_char);
		node = lookup_code(h_table, aux_char, parent_id);
		if(node == NULL){ 
			/* node not found */
			add_code(h_table, aux_char, parent_id, next_id++);
			LOG(DEBUG,"emit code #%d: <\"%c\", %d>", next_id-1, aux_char, parent_id);
			
			ret = bitio_write(b_out, symbol_size, (uint64_t) aux_char); /* emit character */
			if (ret < 0){
				LOG(ERROR, "Write failed: %s", strerror(errno));
				ret = -1;
				goto end;
			}
			
			ret = bitio_write(b_out, id_size, parent_id); /* emit parent_id */
			if (ret < 0){
				LOG(ERROR, "Write failed: %s", strerror(errno));
				ret = -1;
				goto end;
			}			
			parent_id = 0;
			
			/* dictionary is full: clean */
			if(next_id > dictionary_size){
				LOG(WARNING, "Dictionary full!");
				next_id = 1;
				free_table(h_table);
				h_table = create_hash_table(dictionary_size/AVG_CODES_PER_ENTRY);
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
		ret = bitio_write(b_out, symbol_size, (uint64_t) node->character); /* emit character */
		if (ret < 0){
			LOG(ERROR, "Write failed: %s", strerror(errno));
			ret = -1;
			goto end;
		}
						
		ret = bitio_write(b_out, id_size, node->parent_id); /* emit parent_id */
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
	if(b_in != NULL) bitio_close(b_in);
	if(b_out != NULL) bitio_close(b_out);
	return ret;
}
