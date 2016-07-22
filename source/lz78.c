/* TODO: Following the examples in the C bible, I've used 1 as return value
 * when there was an error in the command given by command line, while
 * -1 was used for grave error, e.g memory allocation failed.
 * Finally, 0 was used for success.
 */

#include <ctype.h>		// for isprint
//#include <stdio.h>
#include <stdlib.h>		// for strtol
#include <unistd.h>		// for getopt, access
#include <stdint.h>		//for uintXX_t
#include "common/util.h"
#include "encoder/comp.h"
#include "decoder/decomp.h"

#define DICTIONARY_DEFAULT_SIZE 65536 /* TODO: adjust this value */
#define DICTIONARY_MIN_SIZE 4096 /* TODO: adjust this value */
#define DICTIONARY_MAX_SIZE 4294967295 /* TODO: check this value (2^32 -1)*/
#define SYMBOL_SIZE 8

#define ROLLBACK()									\
	if (input_file_name != NULL){					\
		free(input_file_name);						\
		LOG(DEBUG, "input_file_name deallocated");	\
	}												\
	if (output_file_name != NULL){					\
		free(output_file_name);						\
		LOG(DEBUG, "output_file_name deallocated");	\
	}

void usage(){
	LOG(INFO, "\nUsage: lz78 [OPTION]... [ARGUMENT]...\n"
		"compress or uncompress FILEs using algorithm lz78.\n\n"
		"  -d, decompress\n"
		"  -v, verbose mode\n"
		"  -h, give this help\n"
		"  -l, set dictionary size length. An argument is mandatory\n"
		"  -i, set input file name. An argument is mandatory\n"
		"  -o, set output file name. An argument is mandatory\n\n"
		"At least input file name must be specified.\n\n"
		"Report bugs to <ceafnmcm@gmail.com>.");
}

int safe_filename_cpy(char **file_name, char *optarg){
	uint16_t optlen = strlen(optarg); /* check optlen type */
	if (optlen > 255){
		LOG(INFO, "file name is too long (max 255)");
		return 1;
	}
							
	*file_name = calloc(optlen+1, sizeof(char));
	if (*file_name == NULL){
		LOG(ERROR, "memory allocation failed");
		return -1;
	}
	
	strcpy(*file_name, optarg);
	LOG(DEBUG, "safe_strcpy of %s successfully executed", *file_name);
	return 0;
}

int main (int argc, char **argv){
	extern uint8_t __verbose;
	__verbose = 0;
	uint8_t dflag = 0;	/* decompression flag */
	uint8_t lflag = 0;	/* length of dictionary given flag */
	uint8_t iflag = 0;	/* input file name given flag */
	uint8_t oflag = 0;	/* output file name given flag */
	int64_t aux = 0;
	int ret = 0;
	uint32_t dictionary_size = 0;
	char *input_file_name = NULL;
	char *output_file_name = NULL;
	opterr = 0; /* 0: getopt doesn't print error messages*/	
	int c = 0;
	
	while ((c = getopt (argc, argv, "-d -v -h -l: -i: -o:")) != -1){
		LOG(DEBUG, "c: %c, optarg: %s, optind: %d", c, optarg, optind);
		switch (c){
			case 'd':
				dflag = 1;
				break;
			case 'v':
				__verbose = 1;
				break;
			case 'h':
				usage();
				return 0;				
			case 'l':
				lflag = 1;
				aux = strtol(optarg, NULL, 10);
				/* TODO: CHECK: perform some check on the given length, e.g 
				 * it must be positive, and have a maximum length.
				 * Check must be performed on aux, since the user may issue
				 * a negative value.
				 */
				if (aux < DICTIONARY_MIN_SIZE || aux > DICTIONARY_MAX_SIZE){
					LOG(INFO, "wrong dictionary length. It must be between "
						"%d and %ld", DICTIONARY_MIN_SIZE, DICTIONARY_MAX_SIZE);
					ROLLBACK();
					return 1;
				}
				dictionary_size = aux;
				break;
			case 'i':
				iflag = 1;
				if ((ret = safe_filename_cpy(&input_file_name, optarg)) != 0){
					LOG(INFO, "Error parsing input filename");
					ROLLBACK();
					return ret;
				}
				if( access(input_file_name, F_OK ) == -1 ) {
					LOG(INFO, "file %s doesn't exists", input_file_name);
					ROLLBACK();
					return 1;
					} 		
				break;
			case 'o':
				oflag = 1;
				if ((ret = safe_filename_cpy(&output_file_name, optarg)) != 0){
					LOG(INFO, "Error parsing output filename");
					ROLLBACK();
					return ret;
				}
				break;
			case '?':
				if (optopt == 'l' || optopt == 'i' || optopt == 'o')
					LOG(INFO, "Option -%c requires an argument. "
					"Try 'lz78 -h' for more information", optopt);
				else if (isprint (optopt))
					LOG(INFO, "Unknown option '-%c'. "
					"Try 'lz78-h' for more information", optopt);				
				else
					LOG(INFO, "Unknown option character '\\x%x'"
					"Try 'lz78-h' for more information", optopt);
				ROLLBACK();					
				return 1;
			default:
				ROLLBACK();
				return 1;
		}
	}
	if (iflag == 0){
		LOG(INFO, "Missing input file. Try 'lz78 -h' for more information");
		ROLLBACK();
		return 1;
	}
	if (lflag == 0){
		dictionary_size = DICTIONARY_DEFAULT_SIZE;
	}
	
		LOG(INFO, "The following parameter have been choosen:\n"
			"Compression mode = %s\n"
			"Dictionary size  = %d %s\n"
			"Verbose mode     = %s\n"
			"Input file name  = %s"
			"%s%s",
			 (dflag == 0)? "compression" : "decompression", 
			 dictionary_size, (lflag ==0)? "(default)" : "",
			 (__verbose == 1)? "on" : "off", input_file_name, 
			 (oflag == 1)? "\nOutput file name = " : "",
			 (oflag == 1)? output_file_name : "");
			 
	LOG(INFO, "Starting...");
	if (dflag == 0){
		comp(input_file_name, output_file_name, dictionary_size, SYMBOL_SIZE);
	} else {
		decomp(input_file_name, output_file_name);
	}
}
	


