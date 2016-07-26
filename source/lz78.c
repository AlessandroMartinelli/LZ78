/* TODO: Following the examples in the C bible, I've used 1 as return value
 *  when there was an error in the command given by command line, while
 *  -1 was used for grave error, e.g memory allocation failed.
 *  Finally, 0 was used for success.
 */

#include "lz78.h"

#define DICTIONARY_DEFAULT_LEN 65536	/* TODO: adjust this value */
#define DICTIONARY_MIN_LEN 4096LL		/* TODO: adjust this value */
#define DICTIONARY_MAX_LEN 4294967295LL	/* TODO: check this value (2^32 -1)*/
#define SYMBOL_SIZE 8

#define DECOMP_F	0x80	/* decompression flag */	
#define VERB_F		0x40	/* verbose flag */	
#define HELP_F		0x20	/* help flag */
#define DIC_LEN_F	0x10	/* length of dictionary flag */
#define INPUT_F		0x08	/* input file flag */
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

int comp_init_gstate(struct gstate* state, char* input_file, char* output_file, uint32_t dictionary_len){
	int ret;
	state->b_in = NULL;
	state->b_out = NULL;
	FILE* output_file_ptr = NULL;
	struct stat stat_buf;
	unsigned char checksum[MD5_DIGEST_LENGTH];	
	
	output_file = (output_file == NULL)? path_to_lz78name(input_file) : output_file;
	
	/* Allocation of header_t structure */
	state->header = calloc(1, sizeof(struct header_t));
	if (state->header == NULL){
		errno = ENOMEM;
		LOG(ERROR, "Impossibile to create header_t structure: %s", strerror(errno));
		return -1;
	}
	
	/* Initialization of stat structure */
	stat(input_file, &stat_buf);	
	
	/* Compute checksum of input file */
	csum(input_file, checksum);
	if (checksum == NULL){
		LOG(ERROR, "Impossibile to calculate csum: %s", strerror(errno));
		ret = -1;
	}		
	
	/* Filling of header_t structure */
	*(state->header) = (struct header_t){ 
		stat_buf.st_size,			/* original_size */
		basename(input_file), 		/* input file name */
		checksum,					/* checksum */
		MAGIC,						/* magic number */
		dictionary_len,				/* dictionary_size */
		SYMBOL_SIZE					/* symbol_size */
	};
	
	LOG_BYTES(INFO, state->header->checksum, MD5_DIGEST_LENGTH, 
		"The header structure has been filled in the following way:\n"
		"\tOriginal size    = %ld\n"
		"\tOriginal filname = %s\n"
		"\tMAGIC number     = %d\n"
		"\tdictionary_size  = %u\n"
		"\tsymbol_size      = %u\n"
		"\tchecksum         = ",
		state->header->original_size, state->header->filename,
		state->header->magic_num, state->header->dictionary_size, 
		state->header->symbol_size);
		
	/* Allocation and initialization of bitio structures */
	state->b_in = bitio_open(input_file, READ);
	state->b_out = bitio_open(output_file, WRITE);	
	if(state->b_out == NULL || state->b_in == NULL){
		LOG(ERROR, "Impossibile to allocaate bitio structure: %s", strerror(errno));	
		return -1;
		/*TODO: handle free of bitio */
	}		
		
	/* Writing of the header at the beginning of the file
	 * pointed to by output_file_ptr 
	 */
	output_file_ptr = bitio_get_file(state->b_out);
	ret = header_write(state->header, output_file_ptr);
	if(ret < 0){
		LOG(ERROR, "Header write failed: %s", strerror(errno));
		return -1;
		/* TODO: you know what */
	}		
	return 0;
}

int decomp_init_gstate(struct gstate* state, char* input_file, char* output_file, uint32_t dictionary_len){
	int ret;
	state->b_in = NULL;
	state->b_out = NULL;
	struct stat stat_buf;
	unsigned char checksum[MD5_DIGEST_LENGTH];	
	
	/* Allocation of header_t structure */
	state->header = calloc(1, sizeof(struct header_t));
	if (state->header == NULL){
		errno = ENOMEM;
		LOG(ERROR, "Impossibile to create header_t structure: %s", strerror(errno));
		return -1;
	}
	
	/* Initialization of stat structure */
	stat(input_file, &stat_buf);	
	
	/* Compute checksum of input file */
	csum(input_file, checksum);
	if (checksum == NULL){
		LOG(ERROR, "Impossibile to calculate csum: %s", strerror(errno));
		ret = -1;
	}		
	
	/* Filling of header_t structure */
	*(state->header) = (struct header_t){ 
		stat_buf.st_size,			/* original_size */
		basename(input_file), 		/* input file name */
		checksum,					/* checksum */
		MAGIC,						/* magic number */
		dictionary_len,				/* dictionary_size */
		SYMBOL_SIZE					/* symbol_size */
	};
	
	LOG_BYTES(INFO, state->header->checksum, MD5_DIGEST_LENGTH, 
		"The header structure has been filled in the following way:\n"
		"\tOriginal size    = %ld\n"
		"\tOriginal filname = %s\n"
		"\tMAGIC number     = %d\n"
		"\tdictionary_size  = %u\n"
		"\tsymbol_size      = %u\n"
		"\tchecksum         = ",
		state->header->original_size, state->header->filename,
		state->header->magic_num, state->header->dictionary_size, 
		state->header->symbol_size);
		
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
				"decompressio mode is ignored");
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
		 * If we are in decompressio mode doesn't matter, since this
		 * value won't be used, instead it will be taken from the header 
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
	ret = comp_init_gstate(&state, input_file, output_file, dictionary_len);
	
	return 0;
	
	if ((flag & DECOMP_F) == 0){
		ret = comp(input_file, output_file, dictionary_len, SYMBOL_SIZE);
	} else {
		ret = decomp(input_file, output_file);
	}
	LOG(INFO, "Program terminated with code %d", ret);
	return ret;
}
	


