#include "comp.h"

#define COMP_CLEAN()							\
	if(h_table!=NULL) free_table(h_table);		\
	if(b_in != NULL) bitio_close(b_in);			\
	if(b_out != NULL) bitio_close(b_out);		\
	if(output_file != NULL) free(output_file);

char* path_to_lz78name(char* path){
	char base_name[sizeof(basename(path))+1] = "\0"; /* name without path */
	char *dot_ptr = NULL;	/* pointer at last dot inside a filename */
	char *lz78name = NULL;	/* this one will store the resulting string */
	uint8_t dot_index = 0;	/* position of the last dot inside ta filename */
	
	/* extract the name without path and 
	 * discover the position of the last dot inside the filename 
	 */
	strcpy(base_name, basename(path));
	dot_ptr = strrchr(base_name, '.');
	dot_index = (uint8_t)(dot_ptr - base_name);
	
	/* create and return the new string */	
	lz78name = calloc(dot_index + 5 + 1, sizeof(char));
	if (lz78name == NULL){
		errno = ENOMEM;
		return NULL;
	}
	strncpy(lz78name, base_name, dot_index);
	strcat(lz78name, ".lz78");
	return lz78name;
}	

int comp(char *input_file, char *output_file, uint32_t dictionary_size, uint8_t symbol_size){
	struct bitio *b_out = NULL;
	struct bitio *b_in = NULL;
	struct stat *stat_buf = NULL;	
	struct header_t header;
	hash_table_t *h_table = NULL;
	list_t *node = NULL;
	FILE* input_file_ptr = NULL;
	FILE* output_file_ptr = NULL;	
	uint64_t aux_64;
	char aux_char;
	int ret;
	uint16_t next_id;
	uint16_t parent_id;
	uint16_t id_size;
	unsigned char checksum[MD5_DIGEST_LENGTH];
	
	/* If the caller has not explicitly given a name for the output file,
	*  here we create it, as <name_without_extension>.lz78 
	*/
	if(output_file == NULL) {
		output_file = path_to_lz78name(input_file);
		if (output_file == NULL){
			LOG(ERROR, "%s", strerror(errno));		
			COMP_CLEAN();			
			return -1;
		}	
	}
	
	/* Allocation and initialization of stat structure */
	stat_buf = calloc(1, sizeof(struct stat));
	if (stat_buf == NULL){
		errno = ENOMEM;
		LOG(ERROR, "%s", strerror(errno));	
		COMP_CLEAN();		
		return -1;		
	}
	stat(input_file, stat_buf);	
	
	/* Allocation and initialization of bitio structures */
	b_in = bitio_open(input_file, READ);
	b_out = bitio_open(output_file, WRITE);	
	if(b_out == NULL || b_in == NULL){
		LOG(ERROR, "%s", strerror(errno));	
		COMP_CLEAN();
		return -1;
	}
	
	/* Compute checksum of input file */
	input_file_ptr = bitio_get_file(b_in);	
	csum(input_file_ptr, checksum);
	if (checksum == NULL){
		LOG(ERROR, "%s", strerror(errno));	
		COMP_CLEAN();		
		return -1;
	}
	//checksum[MD5_DIGEST_LENGTH] = '\0';
	//LOG(DEBUG, "checksum vale %s", checksum);
	
	/* Filling of header_t structure */
	header = (struct header_t){ 
		stat_buf->st_size,			/* original_size */
		basename(input_file), 		/* input file name */
		checksum,					/* checksum */
		MAGIC,						/* magic number */
		dictionary_size,			/* dictionary_size */
		symbol_size					/* symbol_size */
	};
	
	LOG(DEBUG, "The header structure has been filled in the following way:\n"
		"\toriginal size: %ld\n"
		"\tinput_file_name: %s\n"
		"\tMAGIC: %d\n"
		"\tdictionary_size: %d\n"
		"\tsymbol_size: %d",
		header.original_size, header.filename,
		header.magic_num, header.dictionary_size, header.symbol_size);
		
	LOG(DEBUG, "checksum: ");
	print_bytes(header.checksum, MD5_DIGEST_LENGTH);
	
	/* Writing of the header at the beginning of the file
	 * pointed to by output_file_ptr 
	 */
	output_file_ptr = bitio_get_file(b_out);
	ret = header_write(&header, output_file_ptr);
	if(ret < 0){
		LOG(ERROR, "Header write has gone wrong");
		COMP_CLEAN();
		return -1;
	}
		
	/* initialization of the hash table (tree abstraction) 
	 * and other variables 
	 */
	h_table = create_hash_table(dictionary_size); //??
	parent_id = 0;
	next_id = 1;
	id_size = (uint16_t) ceil(log(dictionary_size)); /* log base 2 */
	
	while(sizeof(char)*8 == (ret = bitio_read(b_in, sizeof(char)*8, &aux_64))){
	/* as long as characters are available */
		aux_char = (char) aux_64;
		//LOG(DEBUG, "read %d bits (char=%c)", ret, aux_char);
		node = lookup_code(h_table, aux_char, parent_id);
		if(node == NULL){ 
			/* node not found */
			add_code(h_table, aux_char, parent_id, next_id++);
			//LOG(DEBUG,"emit code #%d: <\"%c\", %d>", next_id-1, aux_char, parent_id);
			
			bitio_write(b_out, symbol_size, (uint64_t) aux_char); /* emit character */
			bitio_write(b_out, id_size/*XXX*/, parent_id); /* emit parent_id */
			
			parent_id = 0;
			
			/* dictionary is full: clean */
			if(next_id == dictionary_size){
				LOG(WARNING, "Dictionary full!");
				next_id = 0;
				free_table(h_table);
				h_table = create_hash_table(dictionary_size);
			}
		}
		else{
			//LOG(DEBUG,"code found \"%c\"", aux_char);
			parent_id = node->child_id;
		}
	}
	/* just to be sure it will emits all the characters? */
	if(node != NULL){
		//LOG(DEBUG,"<\"%c\", %d>", node->character, node->parent_id);
		bitio_write(b_out, symbol_size, (uint64_t) aux_char); /* emit character */
		bitio_write(b_out, id_size/*XXX*/, parent_id); /* emit parent_id */
	}
	LOG(INFO,"Compression terminated");
	
	COMP_CLEAN();
	return 0;
}
