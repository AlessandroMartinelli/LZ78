/* TODO: Following the examples in the C bible, I've used 1 as return value
 *  when there was an error in the command given by command line, while
 *  -1 was used for grave error, e.g memory allocation failed.
 *  Finally, 0 was used for success.
 */

#include <ctype.h>		// for isprint
#include <stdlib.h>		// for strtol
#include <unistd.h>		// for getopt, access
#include <stdint.h>		//for uintXX_t
#include <errno.h>		// for using the variabile errno
#include "common/util.h"
#include "common/bitio.h"
#include "common/header.h"
#include "encoder/comp.h"
#include "decoder/decomp.h"

#define DICTIONARY_DEFAULT_SIZE 65536 /* TODO: adjust this value */
#define DICTIONARY_MIN_SIZE 4096 /* TODO: adjust this value */
#define DICTIONARY_MAX_SIZE 4294967295 /* TODO: check this value (2^32 -1)*/
#define SYMBOL_SIZE 8

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

int main (int argc, char **argv){
	extern uint8_t __verbose;
	__verbose = 0;
	uint8_t d_flag = 0;		/* decompression flag */	
	uint8_t v_flag = 0;		/* verbose flag */	
	uint8_t h_flag = 0;		/* help flag */
	uint8_t l_flag = 0;		/* length of dictionary given flag */
	uint8_t i_flag = 0;		/* input file name given flag */
	uint8_t o_flag = 0;		/* output file name given flag */
	uint8_t iu_flag = 0;	/* incorrect usage flag */
	uint8_t mma_flag = 0;	/* missing mandatory argument flag */
	int64_t aux = 0;	
	uint32_t dictionary_size = 0;
	char *input_file_name = NULL;
	char *output_file_name = NULL;
	char *dictionary_size_str = NULL;
	opterr = 0; /* 0: getopt doesn't print error messages*/	
	int c = 0;
	char missing_argument_option;
	
	while ((c = getopt (argc, argv, "-d -v -h -l: -i: -o:")) != -1){
		LOG(DEBUG, "c: %c, optarg: %s, optind: %d", c, optarg, optind);
		switch (c){
			case 'd':
				d_flag = 1;
				break;
			case 'v':
				v_flag = 1;
				break;
			case 'h':
				h_flag = 1;
				break;
			case 'l':
				l_flag = 1;
				dictionary_size_str = optarg;
				break;
			case 'i':
				i_flag = 1;
				input_file_name = optarg;	
				break;
			case 'o':
				o_flag = 1;
				output_file_name = optarg;
				break;
			case '?':
				if (optopt == 'l' || optopt == 'i' || optopt == 'o'){
					mma_flag = 1;
					missing_argument_option = optopt;
				}
				else{ 
					iu_flag = 1;
				}
			default:
				iu_flag = 1;
		}
	}
	if (iu_flag == 1){
		LOG(INFO, "Incorrect usage. Try 'lz78 -h' for more information");
		return 1;
	}
	if (mma_flag == 1){
		LOG(INFO, "Option -%c requires an argument. "
			"Try 'lz78 -h' for more information", missing_argument_option);		
		return 1;
	}
	if (i_flag == 0){
		LOG(INFO, "Missing input file. Try 'lz78 -h' for more information");
		return 1;
	} else {
		if (access(input_file_name, F_OK ) == -1 ) {
			LOG(INFO, "file %s doesn't exists", input_file_name);
			return 1;
		} 	
	}
	if (h_flag == 1){
		usage();
		return 0;
	}
	if (v_flag == 1){
		__verbose = 1;
	}
	if (l_flag == 1){
		aux = strtol(dictionary_size_str, NULL, 10);
		if (aux < DICTIONARY_MIN_SIZE || aux > DICTIONARY_MAX_SIZE){
			LOG(INFO, "wrong dictionary length. It must be between "
				"%d and %ld", DICTIONARY_MIN_SIZE, DICTIONARY_MAX_SIZE);
			return 1;
		}
		dictionary_size = aux;		
	} else {
		dictionary_size = DICTIONARY_DEFAULT_SIZE;
	}
	
	LOG(INFO, "The following parameter have been choosen:\n"
		"Compression mode = %s\n"
		"Dictionary size  = %d %s\n"
		"Verbose mode     = %s\n"
		"Input file name  = %s"
		"%s%s",
		 (d_flag == 0)? "compression" : "decompression", 
		 dictionary_size, (l_flag ==0)? "(default)" : "",
		 (__verbose == 1)? "on" : "off", input_file_name, 
		 (o_flag == 1)? "\nOutput file name = " : "",
		 (o_flag == 1)? output_file_name : "");
		 
	LOG(INFO, "Starting...");
	if (dflag == 0){
		comp(input_file_name, output_file_name, dictionary_size, SYMBOL_SIZE);
	} else {
		decomp(input_file_name, output_file_name);
	}
}
	


