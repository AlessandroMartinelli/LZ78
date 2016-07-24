/* decomp.c
 * 
 * Description----------------------------------------------------------
 * Definition of the behaviors of the compressor.
 * Implementation of the methods declared in "outoder_dictionary.h".
 * ---------------------------------------------------------------------
 */

#include "decomp.h"

#define DECOMP_CLEAN()									\
	if(b_in != NULL) bitio_close(b_in);			\
	if(b_out != NULL) bitio_close(b_out);

void decode(code_t *array, code_t node, struct bitio* b, uint8_t symbol_size){
	if(node.parent_id!=0){
		decode(array, array[node.parent_id - 1], b, symbol_size);
	}
	bitio_write(b, symbol_size, node.character); /* emit character */
	return;
}

int decomp(const char *input_file, const char *output_file){
	struct bitio *b_in = NULL;
	struct bitio *b_out = NULL;
	char aux_char;
	uint32_t size;
	uint32_t dictionary_size;	
	uint64_t num_of_codes;
	uint64_t f_curr;
	uint64_t aux_64;
	uint8_t symbol_size;
	uint16_t id_size;
	uint64_t i;
	unsigned char checksum[MD5_DIGEST_LENGTH];
	//unsigned char *checksum = NULL;
	uint8_t checksum_match = 1;
	FILE* input_file_ptr = NULL;
	struct header_t header;
	struct stat *stat_buf = NULL;
	
	if(input_file == NULL){
		LOG(ERROR,"Input filename missing");
		DECOMP_CLEAN();
		return -1;
	}
	
	b_in = bitio_open(input_file, READ);
	if(b_in == NULL){
		LOG(ERROR, "Impossible to open file %s", input_file);
		DECOMP_CLEAN();
		return -1;
	}
	
	//( ---read the header of b_in
	input_file_ptr = bitio_get_file(b_in);
	if(header_read(&header, input_file_ptr) < 0){
		LOG(ERROR, "Header read gone wrong");
		DECOMP_CLEAN();
		return -1;
	}
	
	LOG(DEBUG, "The header structure has been filled in the following way:\n"
		"\toriginal size: %ld\n"
		"\tinput_file_name %s\n"
		"\tMAGIC %d\n"
		"\tdictionary_size %d\n"
		"\tsymbol_size %d",
		header.original_size, header.filename,
		header.magic_num, header.dictionary_size, header.symbol_size);
		
	LOG(DEBUG, "checksum: ");
	print_bytes(header.checksum, MD5_DIGEST_LENGTH);
	
	if(header.magic_num != MAGIC){
		LOG(ERROR,"Wrong decompression/wrong file");
		DECOMP_CLEAN();
		return -1;
	}
	dictionary_size = header.dictionary_size;
	id_size = (uint16_t) ceil(log(dictionary_size)); /* log base 2 */
	symbol_size = header.symbol_size;
	//---)
	
	b_out = bitio_open(output_file==NULL ? header.filename : output_file, WRITE);
	if(b_out == NULL){
		LOG(ERROR, "Output file impossible to be open");
		DECOMP_CLEAN();
		return -1;
	}
	
	/* get the number of codes */
	f_curr = ftell(input_file_ptr);
	fseek(input_file_ptr, 0L, SEEK_END);
	num_of_codes = ftell(input_file_ptr) - f_curr;
	fseek(input_file_ptr, 0L, f_curr);
	num_of_codes /= symbol_size+id_size;     /* normalize the number of array
													      * elements by the code dimension*/
	/* the dictionary is clean every dictionary_size codes */
	size = (num_of_codes <= dictionary_size)? num_of_codes : dictionary_size;
	LOG(INFO,"size: %d\n",size);
	
	code_t nodes[size];
	
	for(i=0; i<num_of_codes; i++){
		if(bitio_read(b_in, id_size, &aux_64) == id_size &&
			bitio_read(b_in, symbol_size, (uint64_t*) &aux_char) == symbol_size){
			nodes[i%size].character = (char) aux_char;
			nodes[i%size].parent_id = aux_64;
			LOG(INFO, "<\"%c\", %lu>\n", nodes[i%size].character, nodes[i%size].parent_id);
			decode(nodes, nodes[i%size], b_out, symbol_size);
		}
		else{
			LOG(ERROR,"Code unreadable");
		DECOMP_CLEAN();
		return -1;
		}
		/*clean of the dictionary not needed*/
	}
	
	/* compare headers */
	FILE* f_out = bitio_get_file(b_out);
	csum(f_out, checksum);
	
	int ret = strncmp(checksum, header.checksum, MD5_DIGEST_LENGTH);
	if (ret == 0){
		LOG(INFO, "checksum successfully verified");
	}
	
	for(i=0; i<MD5_DIGEST_LENGTH; i++){
		if(checksum[i] == header.checksum[i]) continue;
		LOG(WARNING, "Checksum error");
		checksum_match = 0;
		break;
	}
	if(checksum_match){
		fstat(fileno(f_out),stat_buf);
		if(stat_buf->st_size != header.original_size)
			LOG(WARNING,"Original size error");
	}
	DECOMP_CLEAN();
	return 0;
}
