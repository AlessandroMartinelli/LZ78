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
#include "encoder/comp.h"
#include "decoder/decomp.h"

#define DICTIONARY_DEFAULT_LEN 65536 /* TODO: adjust this value */
#define DICTIONARY_MIN_LEN 4096LL /* TODO: adjust this value */
#define DICTIONARY_MAX_LEN 4294967295LL /* TODO: check this value (2^32 -1)*/
#define SYMBOL_SIZE 8

#define DECOMP_F	0x80	/* decompression flag */	
#define VERB_F		0x40	/* verbose flag */	
#define HELP_F		0x20	/* help flag */
#define DIC_LEN_F	0x10	/* length of dictionary given flag */
#define INPUT_F		0x08	/* input file name given flag */
#define OUTPUT_F	0x04	/* output file name given flag */
#define MMA_F		0x02	/* incorrect usage flag */
#define IU_F		0x01	/* missing mandatory argument flag */

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
	uint8_t flag = 0;
	int64_t aux = 0;	
	uint32_t dictionary_len = 0;
	char *input_file_name = NULL;
	char *output_file_name = NULL;
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
				input_file_name = optarg;	
				break;
			case 'o':
				flag |= OUTPUT_F;
				output_file_name = optarg;
				break;
			case '?':
				if (optopt == 'l' || optopt == 'i' || optopt == 'o'){
					flag |= MMA_F;
					missing_mandatory_argument = optopt;
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
	if (flag == HELP_F){
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
	} else {
		if (access(input_file_name, F_OK ) == -1 ) {
			LOG(INFO, "file %s doesn't exists", input_file_name);
			return 1;
		} 
	}
	if ((flag & VERB_F) != 0){
		__verbose = 1;
	}
	if ((flag & DIC_LEN_F) != 0){
		aux = strtol(dictionary_len_str, NULL, 10);
		if (aux < DICTIONARY_MIN_LEN || aux > DICTIONARY_MAX_LEN){
			LOG(INFO, "wrong dictionary length. It must be between "
				"%lld and %lld", DICTIONARY_MIN_LEN, DICTIONARY_MAX_LEN);
			return 1;
		}
		dictionary_len = aux;		
	} else {
		dictionary_len = DICTIONARY_DEFAULT_LEN;
	}
	
	LOG(INFO, "The following parameter have been choosen:\n"
		"Compression mode = %s\n"
		"Dictionary len   = %d %s\n"
		"Verbose mode     = %s\n"
		"Input file name  = %s"
		"%s%s",
		 ((flag & DECOMP_F) == 0)? "compression" : "decompression", 
		 dictionary_len, ((flag & DIC_LEN_F) == 0)? "(default)" : "",
		 ((flag & VERB_F) != 0)? "on" : "off", input_file_name, 
		 ((flag & OUTPUT_F) != 0)? "\nOutput file name = " : "",
		 ((flag & OUTPUT_F) != 0)? output_file_name : "");
		 
	LOG(INFO, "Starting...");
	if ((flag & DECOMP_F) != 0){
		comp(input_file_name, output_file_name, dictionary_len, SYMBOL_SIZE);
	} else {
		decomp(input_file_name, output_file_name);
	}
	return 0;
}
	


