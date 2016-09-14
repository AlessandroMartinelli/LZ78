/* TODO: Following the examples in the C bible, I've used 1 as return value
 *  when there was an error in the command given by command line, while
 *  -1 was used for grave error, e.g memory allocation failed.
 *  Finally, 0 was used for success.
 */

#include "lz78.h"

#define DICTIONARY_DEFAULT_LEN 65536		/* ~ 2 levels */
#define DICTIONARY_MIN_LEN 16777216LL		/* ~ 3 levels */
#define DICTIONARY_MAX_LEN 4294967296LL	/* ~ 4 levels */
#define SYMBOL_SIZE 8

#define DECOMP_F	0x80	/* decompression flag */
#define VERB_F		0x40	/* verbose flag */
#define HELP_F		0x20	/* help flag */
#define DIC_LEN_F	0x10	/* length of dictionary flag */
#define INPUT_F	0x08	/* input file flag */
#define OUTPUT_F	0x04	/* output file flag */
#define MMA_F		0x02	/* missing mandatory argument flag */
#define IU_F		0x01	/* incorrect usage flag */

void usage(){
	printf("Usage: lz78 [OPTION]... [ARGUMENT]...\n"
		"Compress or uncompress FILEs using algorithm lz78 (by default, compress FILEs).\n\n"
		"  -d, decompress\n"
		"  -v, verbose mode\n"
		"  -h, give this help\n"
		"  -l, set dictionary length. An argument is mandatory\n"
		"  -i, set input file name. An argument is mandatory\n"
		"  -o, set output file name. An argument is mandatory\n\n"
		"At least input file name must be specified.\n\n"
		"Report bugs to <ceafnmcm AT gmail DOT com>.\n");
}

char* path_to_lz78name(char* path){
	char *base_name;
	char *dot_ptr = NULL;	/* pointer at last dot inside a filename */
	char *lz78name = NULL;	/* this one will store the resulting string */
	uint8_t dot_index = 0;	/* position of the last dot inside ta filename */

	/* Extract the name without path and discover
	 * the position of the last dot inside the filename
	 */
	base_name = basename(path);
	dot_ptr = strrchr(base_name, '.');
	dot_index = (uint8_t)(dot_ptr - base_name);

	/* Create and return the new string */
	lz78name = calloc(dot_index + 5 + 1, sizeof(char));
	if (lz78name == NULL){
		errno = ENOMEM;
		return NULL;
	}
	strncpy(lz78name, base_name, dot_index);
	strcat(lz78name, ".lz78");
	return lz78name;
}

void clean_bitio(struct gstate* state){
	if(state){
		if(state->b_in) bitio_close(state->b_in);
		if(state->b_out) bitio_close(state->b_out);
		state->b_in = NULL;
		state->b_out = NULL;
	}
}

void clean_state(struct gstate* state){
	if(state){
		if(state->header){
			if(state->header->filename) free(state->header->filename);
			if(state->header->checksum) free(state->header->checksum);
			state->header->filename = NULL;
			state->header->checksum = NULL;
			free(state->header);
			state->header = NULL;
		}
		clean_bitio(state);
	}
	state = NULL;
	
	/* The input file is always explicitly given, so there is no need
	 * for explicitly freeing it. About the output_file:
	 * If the output_file name has been explicitly given, there is no need
	 * to manually free it. Even though that is not the case, if we are in
	 * decompression mode the output_file name has been allocated in the
	 * header_read, so it will be deallocated when the gstate structure
	 * will be deallocated. So, we must explicitly free it only in the
	 * remaining following situation:
	 */
	/*if ((flag & OUTPUT_F) == 0){
		if ((flag & DECOMP_F) == 0){
			free(output_file);
		}
	}*/
}

int comp_chooser(struct gstate* state, char* output_file){
	struct stat stat_buf;
	if (stat(output_file, &stat_buf) == -1){
		LOG(ERROR, "Impossible to compute statistics: %s", strerror(errno));
		return -1;
	}
	if(state->header->original_size < (uint64_t) stat_buf.st_size){
		LOG(DEBUG,"Plain compression failed. Proceeding with fake compression!");
		return 1; /* compression ineffective */
	}
	
	LOG(DEBUG,"Plain compression worked well!");
	return 0; /* file compressed properly */
}

int decomp_chooser(struct gstate* state){
	switch(state->header->magic_num){
		case MAGIC_NORMAL:
			LOG(DEBUG,"Plain decompression!");
			return 0; /* plain compression */
		case MAGIC_FAKE:
			LOG(DEBUG,"Fake decompression!");
			return 1; /* fake compression */
		default:
			LOG(ERROR, "This file is not valid for the decompression");
			return -1;
	}
}

int comp_init_gstate(struct gstate* state, char* input_file, char* output_file, uint32_t dictionary_len){
	int ret, name_len;
	state->b_in = NULL;
	state->b_out = NULL;
	FILE* output_file_ptr = NULL;
	struct stat stat_buf;
	char* bname = NULL;
	unsigned char* checksum;

	/* Allocation of header_t structure */
	state->header = calloc(1, sizeof(struct header_t));
	if (state->header == NULL){
		errno = ENOMEM;
		LOG(ERROR, "Impossible to create header_t structure: %s", strerror(errno));
		return -1;
	}

	/* Initialization of stat structure (stat_buf) */
	ret = stat(input_file, &stat_buf);
	if (ret == -1){
		LOG(ERROR, "Impossible to create calculate statistics: %s", strerror(errno));
		return -1;
	}

	/* Compute checksum of input file and put it in "checksum" */
	checksum = (unsigned char*)calloc(1, MD5_DIGEST_LENGTH);
	if (checksum == NULL){
		errno = ENOMEM;
		LOG(ERROR, "Impossible to allocate the checksum: %s", strerror(errno));
		ret = -1;
	}
	csum(input_file, checksum);
	if (checksum == NULL){
		LOG(ERROR, "Impossible to compute checksum: %s", strerror(errno));
		ret = -1;
	}
	
	/* Copy original filename into the heap */
	name_len = strlen(basename(input_file));
	bname = calloc(name_len+1, sizeof(char));
	strncpy(bname, basename(input_file), name_len);

	/* Filling of header_t structure */
	*(state->header) = (struct header_t){
		stat_buf.st_size,			/* original_size */
		bname, 						/* input file name */
		checksum,					/* checksum */
		MAGIC_NORMAL,						/* magic number */
		dictionary_len,			/* dictionary_size */
		SYMBOL_SIZE					/* symbol_size */
	};

	LOG_BYTES(INFO, state->header->checksum, MD5_DIGEST_LENGTH,
		"The header structure has been filled in the following way:\n"
		"\tOriginal size    = %llu\n"
		"\tOriginal filname = %s\n"
		"\tMAGIC number     = %u\n"
		"\tdictionary_len  = %u\n"
		"\tsymbol_size      = %hhu\n"
		"\tchecksum         = ",
		state->header->original_size, state->header->filename,
		state->header->magic_num, state->header->dictionary_len,
		state->header->symbol_size);
	
	/* Allocation and initialization of bitio structures */
	state->b_in = bitio_open(input_file, READ);
	state->b_out = bitio_open(output_file, WRITE);
	if(state->b_out == NULL || state->b_in == NULL){
		LOG(ERROR, "Impossible to allocate bitio structure: %s", strerror(errno));
		return -1;
	}

	/* Writing of the header at the beginning of the file
	 * pointed to by output_file_ptr
	 */
	output_file_ptr = bitio_get_file(state->b_out);
	ret = header_write(state->header, output_file_ptr);
	if(ret < 0){
		LOG(ERROR, "Header write failed: %s", strerror(errno));
		return -1;
	}

	output_file_ptr = NULL;
	return 0;
}

int decomp_init_gstate(struct gstate* state, char* input_file, char* output_file){
	int ret;
	state->b_in = NULL;
	state->b_out = NULL;
	FILE* input_file_ptr = NULL;

	/* Allocation of header_t structure */
	state->header = calloc(1, sizeof(struct header_t));
	if (state->header == NULL){
		errno = ENOMEM;
		LOG(ERROR, "Impossible to create header_t structure: %s", strerror(errno));
		return -1;
	}

	/* Allocation and initialization of bitio read structure */
	state->b_in = bitio_open(input_file, READ);
	if(state->b_in == NULL){
		LOG(ERROR, "Impossible to allocate bitio structure: %s", strerror(errno));
		return -1;
	}

	/* We take the input file pointer from the bitio read structure,
	 * then we retrieve the file dimension. The choice of retrieving the
	 * file dimension here isn'n very appropriate, but it is useful
	 * since here, before reading the header, is easier to scan the
	 * file up and down without any concern.
	 */
	input_file_ptr = bitio_get_file(state->b_in);

	/* reading of the header */
	ret = header_read(state->header, input_file_ptr);
	if (ret < 0){
		LOG(ERROR, "Header read failed: %s", strerror(errno));
		return -1;
	}

	LOG_BYTES(INFO, state->header->checksum, MD5_DIGEST_LENGTH, "The header structure has been filled in the following way:\n"
		"\tOriginal size    = %llu\n"
		"\tOriginal filname = %s\n"
		"\tMAGIC number     = %u\n"
		"\tdictionary_len  = %u\n"
		"\tsymbol_size      = %hhu\n"
		"\tchecksum         = ",
		state->header->original_size, state->header->filename,
		state->header->magic_num, state->header->dictionary_len, state->header->symbol_size);

	/* If the caller has not explicitly given a name for the output file,
	*  we grab it from the header we've just read
	*/
	state->header->filename = (output_file == NULL)? state->header->filename : output_file;

	/* Allocation and initialization of output bitio structure */
	state->b_out = bitio_open(state->header->filename, WRITE);
	if(state->b_out == NULL){
		LOG(ERROR, "Impossible to allocate bitio structure: %s", strerror(errno));
		return -1;
	}

	return 0;
}


int main (int argc, char **argv){
	extern uint8_t __verbose;
	__verbose = 0;
	int ret = 0;
	uint8_t flag = 0;
	int64_t aux = 0;
	uint32_t dictionary_len = 0;
	char *input_file = NULL;
	char *output_file = NULL;
	char *dictionary_len_str = NULL;
	opterr = 0; /* 0: getopt doesn't print error messages*/
	int c = 0;
	char missing_mandatory_argument;

	while ((c = getopt (argc, argv, "-d -v -h -l: -i: -o:")) != -1){
		switch (c){
			case 'd':
				flag |= DECOMP_F;
				break;
			case 'v':
				flag |= VERB_F;
				break;
			case 'h':
				flag |= HELP_F;
				break;
			case 'l':
				flag |= DIC_LEN_F;
				dictionary_len_str = optarg;
				break;
			case 'i':
				flag |= INPUT_F;
				input_file = optarg;
				break;
			case 'o':
				flag |= OUTPUT_F;
				output_file = optarg;
				break;
			case '?':
				if (optopt == 'l' || optopt == 'i' || optopt == 'o'){
					flag |= MMA_F;
					missing_mandatory_argument = optopt;
					break;
				}
				/* break omitted intentionally */
			default:
				flag |= IU_F;
		}
	}
	if ((flag & IU_F) != 0){
		LOG(INFO, "Incorrect usage. Try 'lz78 -h' for more information");
		return 1;
	}
	if ((flag & HELP_F) != 0){
		if (flag != HELP_F){
			/* help_flag is setted, but it is not the only setted flag */
			LOG(INFO, "Incorrect usage. Try 'lz78 -h' for more information");
			return 1;
		}
		usage();
		return 0;
	}
	if ((flag & MMA_F) != 0){
		LOG(INFO, "Option -%c requires an argument. "
			"Try 'lz78 -h' for more information", missing_mandatory_argument);
		return 1;
	}
	if ((flag & INPUT_F) == 0){
		LOG(INFO, "Missing input file. Try 'lz78 -h' for more information");
		return 1;
	}
	if (access(input_file, F_OK ) == -1 ) {
		LOG(INFO, "file %s doesn't exists", input_file);
		return 1;
	}
	if ((flag & VERB_F) != 0){
		__verbose = 1;
	}
	if ((flag & DIC_LEN_F) != 0){
		/* Dictionary length has been given */
		if ((flag & DECOMP_F) !=0){
			/* In decompression mode, giving -l option has no effect */
			LOG(WARNING, "Dictionary length given as argument in "
				"decompression mode is ignored");
		} else {
			aux = strtol(dictionary_len_str, NULL, 10);
			if (aux < DICTIONARY_MIN_LEN || aux > DICTIONARY_MAX_LEN){
				LOG(INFO, "wrong dictionary length. It must be between "
					"%lld and %lld", DICTIONARY_MIN_LEN, DICTIONARY_MAX_LEN);
				return 1;
			}
			dictionary_len = aux;
		}
	} else {
		/* If dictionary length has not been given, a default value is used.
		 * If we are in decompression mode it doesn't matter, since this
		 * value won't be used (it will be taken from the header).
		 */
		dictionary_len = DICTIONARY_DEFAULT_LEN;
	}
	if ((flag & DECOMP_F) == 0){
		LOG(INFO, "The following parameter have been choosen:\n"
			"\tDictionary len   = %d %s\n"
			"\tVerbose mode     = %s\n"
			"\tInput file       = %s"
			"%s%s",
			dictionary_len, ((flag & DIC_LEN_F) == 0)? "(default)" : "",
			((flag & VERB_F) != 0)? "on" : "off", input_file,
			((flag & OUTPUT_F) != 0)? "\n\tOutput file      = " : "",
			((flag & OUTPUT_F) != 0)? output_file : "");
	}
	
	
	LOG(INFO, "Starting...");
	struct gstate state;

	if ((flag & DECOMP_F) == 0){ /* Compressor mode */
		/* If the caller has not explicitly given a name for the output file,
		 *  here we create it, as <name_without_path_and_extension>.lz78
		 */
		output_file = (output_file == NULL) ?
			path_to_lz78name(input_file) : output_file;
		
		ret = comp_init_gstate(&state, input_file, output_file, dictionary_len);
		if (ret == -1) goto end;
		
		ret = comp(&state);
		if (ret == -1) goto end;
		
		clean_bitio(&state);
		ret = comp_chooser(&state, output_file);
		if(ret == 1){
			state.header->magic_num = MAGIC_FAKE;
			fake_comp(&state, input_file, output_file);
		}
		
	} else { /* Decompressor mode */
		ret = decomp_init_gstate(&state, input_file, output_file);
		if (ret == -1) goto end;
		
		ret = decomp_chooser(&state);
		if(ret == 0){
			ret = decomp(&state);
		} else if(ret == 1){
			ret = fake_decomp(&state);
		}
		
		clean_bitio(&state);
		ret = decomp_check(&state);
	}

end:
	clean_state(&state);
	
	LOG(INFO, "Program terminated with code %d", ret);
	return ret;
}



