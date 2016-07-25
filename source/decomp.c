/* decomp.c
 * 
 * Description----------------------------------------------------------
 * Definition of the behaviors of the compressor.
 * Implementation of the methods declared in "outoder_dictionary.h".
 * ---------------------------------------------------------------------
 */

#include "decomp.h"

#define DECOMP_CLEAN()								\
	if(b_in != NULL) bitio_close(b_in);			\
	if(b_out != NULL) bitio_close(b_out);

void decode(code_t *array, code_t node, struct bitio* b, uint8_t symbol_size){
	//LOG(DEBUG, "\tDecode <\"%c\", %lu>", node.character, node.parent_id);
	if(node.parent_id!=0){
		decode(array, array[node.parent_id - 1], b, symbol_size);
	}
	//LOG(DEBUG, "emit <\"%c\", %lu>", node.character, node.parent_id);
	bitio_write(b, symbol_size, node.character); /* emit character */
	return;
}

int decomp(const char *input_file, const char *output_file){
	struct bitio *b_in = NULL;
	struct bitio *b_out = NULL;
	int ret_symbol, ret_id;
	char aux_char;
	uint32_t size;
	uint32_t dictionary_size;
	uint64_t num_of_codes;
	uint64_t f_dim;
	uint64_t aux_64;
	uint8_t symbol_size;
	uint16_t id_size;
	uint64_t i;
	unsigned char checksum[MD5_DIGEST_LENGTH];
	uint8_t size_match = 1;
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
	
	//(--- read the header of b_in
	input_file_ptr = bitio_get_file(b_in);
	fseek(input_file_ptr, 0L, SEEK_END);
	f_dim = ftell(input_file_ptr);
	fseek(input_file_ptr, 0L, SEEK_SET);
	
	if(header_read(&header, input_file_ptr) < 0){
		LOG(ERROR, "Header read gone wrong");
		DECOMP_CLEAN();
		return -1;
	}
	
	LOG(INFO, "The header structure has been filled in the following way:\n"
		"\tOriginal size    = %ld\n"
		"\tOriginal filname = %s\n"
		"\tMAGIC number     = %d\n"
		"\tdictionary_size  = %d\n"
		"\tsymbol_size      = %d",
		header.original_size, header.filename,
		header.magic_num, header.dictionary_size, header.symbol_size);
	//LOG(DEBUG, "checksum: ");
	//print_bytes((char*) header.checksum, MD5_DIGEST_LENGTH);
	if(header.magic_num != MAGIC){
		LOG(ERROR,"Wrong decompression/wrong file");
		DECOMP_CLEAN();
		return -1;
	}
	dictionary_size = header.dictionary_size;
	id_size = ceil_log2(dictionary_size); /* log base 2 */
	symbol_size = header.symbol_size;
	//---)
	
	b_out = bitio_open(output_file==NULL ? header.filename : output_file, WRITE);
	if(b_out == NULL){
		LOG(ERROR, "Output file impossible to be open");
		DECOMP_CLEAN();
		return -1;
	}
	
	/* get the number of codes */
	num_of_codes = f_dim - ftell(input_file_ptr);
	num_of_codes *= 8; // bytes to bits
	num_of_codes /= symbol_size+id_size;	/* normalize the number of array
														 * elements by the code dimension*/
	LOG(INFO,"num_of_codes: %lu",num_of_codes);
	
	/* the dictionary is clean every dictionary_size codes */
	size = (num_of_codes <= dictionary_size)? num_of_codes : dictionary_size;
	LOG(INFO,"dictionary size: %d",size); 
	code_t nodes[size];
	
	for(i=0; i<num_of_codes; i++){
		/* clean of the dictionary not needed */
		if(i!=0 && i%dictionary_size == 0) LOG(WARNING, "Clean the dictionary");
		
		ret_symbol = bitio_read(b_in, symbol_size, &aux_64);
		aux_char = (char) aux_64;
		ret_id = bitio_read(b_in, id_size, &aux_64);
		
		if(ret_symbol == symbol_size && ret_id == id_size){
			nodes[i%dictionary_size].character = (char) aux_char;
			nodes[i%dictionary_size].parent_id = aux_64;
			LOG(DEBUG, "Code read #%lu: <\"%c\", %lu>", i%dictionary_size,
				nodes[i%dictionary_size].character,
				nodes[i%dictionary_size].parent_id);
			decode(nodes, nodes[i%dictionary_size], b_out, symbol_size);
		}
		else{
			nodes[i%dictionary_size].character = (char) aux_char;
			nodes[i%dictionary_size].parent_id = aux_64;
			LOG(ERROR,"Code unreadable <\"%c\", %lu>",
				nodes[i%dictionary_size].character,
				nodes[i%dictionary_size].parent_id);
			decode(nodes, nodes[i%dictionary_size], b_out, symbol_size);
			DECOMP_CLEAN();
			return -1;
		}
	}
	
	/* flush bitio buffer*/
	bitio_close(b_out);
	b_out = NULL;

	/* compare file size */
	stat_buf = calloc(1, sizeof(struct stat));
	if (stat_buf == NULL){
		errno = ENOMEM;
		LOG(ERROR, "%s", strerror(errno));
		DECOMP_CLEAN();
		size_match = 0;
	} else {
		stat(output_file,stat_buf);
		if(stat_buf == NULL){
			LOG(ERROR, "Cannot compute stats on %s", output_file);
			DECOMP_CLEAN();
			size_match = 0;
		} else {
			if((uint64_t)stat_buf->st_size != header.original_size){
				LOG(ERROR,"Original size error");
				DECOMP_CLEAN();
				size_match = 0;
			} else {
				LOG(INFO, "size OK!");
			}
		}
	}
	
	if(size_match){
		/* compare checksum */
		csum(output_file, checksum);
		if (checksum == NULL){
			LOG(ERROR, "%s", strerror(errno));
			DECOMP_CLEAN();
			return -1;
		}
		//print_bytes((char*) checksum, MD5_DIGEST_LENGTH);
		//print_bytes((char*) header.checksum, MD5_DIGEST_LENGTH);
	
		if(memcmp((char*)checksum, (char*)header.checksum,MD5_DIGEST_LENGTH) != 0){
			LOG(ERROR, "Checksum error: %s", strerror(errno));
		}
		else{
			LOG(INFO, "Checksum OK!");
		}
	}
	
	/* Cannot use goto because of vector nodes (with variably modified type) */
	DECOMP_CLEAN();
	return 0;
}
