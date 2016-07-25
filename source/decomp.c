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
	if(b_out != NULL) bitio_close(b_out);		\
	header_read_free(&header);

int decode(code_t *array, code_t node, struct bitio* b, uint8_t symbol_size){
	int ret = 0;
	//LOG(DEBUG, "\tDecode <\"%c\", %lu>", node.character, node.parent_id);
	if(node.parent_id!=0){
		decode(array, array[node.parent_id - 1], b, symbol_size);
	}
	//LOG(DEBUG, "emit <\"%c\", %lu>", node.character, node.parent_id);
	ret = bitio_write(b, symbol_size, node.character); /* emit character */
	if (ret < 0){
		return -1;
	}
	
	return 0;
}

int decomp(const char *input_file, const char *output_file){
	struct bitio *b_in = NULL;
	struct bitio *b_out = NULL;
	int ret_symbol;
	int ret_id;
	int ret = 0;
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
		LOG(ERROR, "Impossible to open file %s: %s", input_file, strerror(errno));
		DECOMP_CLEAN();
		return -1;
	}
	
	//( ---read the header of b_in
	input_file_ptr = bitio_get_file(b_in);
	fseek(input_file_ptr, 0L, SEEK_END);
	f_dim = ftell(input_file_ptr);
	fseek(input_file_ptr, 0L, SEEK_SET);
	
	ret = header_read(&header, input_file_ptr);
	if (ret < 0){
		LOG(ERROR, "Header read failed: %s", strerror(errno));
		DECOMP_CLEAN();
		return -1;
	}
	
	LOG_BYTES(INFO, header.checksum, MD5_DIGEST_LENGTH, "The header structure has been filled in the following way:\n"
		"\tOriginal size    = %ld\n"
		"\tOriginal filname = %s\n"
		"\tMAGIC number     = %d\n"
		"\tdictionary_size  = %d\n"
		"\tsymbol_size      = %d\n"
		"\tchecksum         = ",
		header.original_size, header.filename,
		header.magic_num, header.dictionary_size, header.symbol_size);
	
	if(header.magic_num != MAGIC){
		LOG(ERROR,"Wrong decompression/wrong file");
		DECOMP_CLEAN();
		return -1;
	}
	
	dictionary_size = header.dictionary_size;
	id_size = ceil_log2(dictionary_size); /* log base 2 */
	symbol_size = header.symbol_size;
	//---)
	
	output_file = (output_file == NULL) ? header.filename : output_file;
	
	b_out = bitio_open(output_file, WRITE);
	if(b_out == NULL){
		LOG(ERROR, "Impossibile to open %s: %s", output_file, strerror(errno));
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
	LOG(INFO,"Dictionary size: %d", size); 
	code_t nodes[size];
	
	for(i=0; i<num_of_codes; i++){
		/* clean of the dictionary not needed */
		if(i!=0 && i%dictionary_size == 0) LOG(WARNING, "Clean the dictionary");
		
		ret_symbol = bitio_read(b_in, symbol_size, &aux_64);
		if (ret_symbol < 0){
			LOG(ERROR, "Read failed: %s", strerror(errno));
			DECOMP_CLEAN();
			return -1;
		}
		
		aux_char = (char) aux_64;
		ret_id = bitio_read(b_in, id_size, &aux_64);
		if (ret_id < 0){
			LOG(ERROR, "Read failed: %s", strerror(errno));
			DECOMP_CLEAN();
			return -1;
		}		
		
		if(ret_symbol == symbol_size && ret_id == id_size){
			nodes[i%dictionary_size].character = (char) aux_char;
			nodes[i%dictionary_size].parent_id = aux_64;
			LOG(DEBUG, "Code read #%lu: <\"%c\", %lu>", i%dictionary_size,
				nodes[i%dictionary_size].character,
				nodes[i%dictionary_size].parent_id);
			ret = decode(nodes, nodes[i%dictionary_size], b_out, symbol_size);
			if (ret < 0){
				LOG(ERROR, "Decode failed: %s", strerror(errno));
				DECOMP_CLEAN();
				return -1;
			}				
		}
		else{
			nodes[i%dictionary_size].character = (char) aux_char;
			nodes[i%dictionary_size].parent_id = aux_64;
			LOG(ERROR,"Code unreadable <\"%c\", %lu>",
				nodes[i%dictionary_size].character,
				nodes[i%dictionary_size].parent_id);
			ret = decode(nodes, nodes[i%dictionary_size], b_out, symbol_size);
			if (ret < 0){
				LOG(ERROR, "Decode failed: %s", strerror(errno));
			}
			DECOMP_CLEAN();
			return -1;
		}
	}
	
	/* flush bitio buffer*/
	ret = bitio_close(b_out);
	if (ret < 0){
			LOG(ERROR, "Close failed: %s", strerror(errno));
			DECOMP_CLEAN();
			return -1;
	}		
	b_out = NULL;

	/* compare file size */
	stat_buf = calloc(1, sizeof(struct stat));
	if (stat_buf == NULL){
		errno = ENOMEM;
		LOG(ERROR, "%s", strerror(errno));
		DECOMP_CLEAN();
		return -1;
	} 
	
	stat(output_file, stat_buf);
	if(stat_buf == NULL){
		LOG(ERROR, "Cannot compute stats on %s", output_file);
		DECOMP_CLEAN();
		return -1;
	}
	
	if((uint64_t)stat_buf->st_size != header.original_size){
		LOG(ERROR,"Original size error");
		DECOMP_CLEAN();
		return -1;
	}
	LOG(INFO, "Size match: OK!");
	
	/* compare checksum */
	csum(output_file, checksum);
	if (checksum == NULL){
		LOG(ERROR, "Checksum calculation failed: %s", strerror(errno));
		DECOMP_CLEAN();
		return -1;
	}		
	LOG_BYTES(DEBUG, header.checksum, MD5_DIGEST_LENGTH, "Received checksum: ");
	LOG_BYTES(DEBUG, checksum, MD5_DIGEST_LENGTH, "Calculated checksum: ");			
	
	if(memcmp((char*)checksum, (char*)header.checksum,MD5_DIGEST_LENGTH) != 0){
		LOG(ERROR, "Checksum error: received and calculated checksum don't match");
		DECOMP_CLEAN();
		return -1;		
	} 
	LOG(INFO, "Checksum match: OK!");
	
	/* Cannot use goto because of vector nodes (with variably modified type) */
	DECOMP_CLEAN();
	return 0;
}
