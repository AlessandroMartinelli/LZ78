/* TODO: Following the examples in the C bible, I've used 1 as return value
 * when there was an error in the command given by command line, while
 * -1 was used for grave error, e.g memory allocation failed.
 * Finally, 0 was used for success.
 */

#include <ctype.h>		// for isprint
//#include <stdio.h>
#include <stdlib.h>		// for strtol
#include <unistd.h>		// for getopt
#include <stdint.h>		//for uintXX_t
#include "common/util.h"

#define DICTIONARY_DEFAULT_SIZE 65536 /* TODO: adjust this value */
#define DICTIONARY_MIN_SIZE 4096 /* TODO: adjust this value */
#define DICTIONARY_MAX_SIZE 4294967295 /* TODO: check this value (2^32 -1)*/

void usage(){
	printf("Usage:\t...");
}

int safe_strcpy(char **file_name, char *optarg){
	uint16_t optlen = strlen(optarg); /* check optlen type */
	if (optlen > 255){
		LOG(INFO, "file name is too long (max 255)");
		return 1;
	}
							
	*file_name = calloc(optlen+1, 1);
	if (*file_name == NULL){
		LOG(ERROR, "memory allocation failed");
		return -1;
	}
	strcpy(*file_name, optarg);
	LOG(DEBUG, "safe_strcpy of %s successfully executed", *file_name);
	return 0;
}

int main (int argc, char **argv){
	__verbose = 0; /* global variable */
	uint8_t dflag = 0;
	uint8_t lflag = 0;
	uint8_t iflag = 0;
	uint8_t oflag = 0;
	int64_t aux = 0;
	int ret = 0;
	uint32_t dictionary_size = 0;
	char *input_file_name = NULL;
	char *output_file_name = NULL;
	int c = 0;
	opterr = 0;
	
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
					return 1;
				}
				dictionary_size = aux;
				break;
			case 'i':
				iflag = 1;
				if ((ret = safe_strcpy(&input_file_name, optarg)) != 0){
					LOG(INFO, "Error parsing input filename"); /* TODO: it's ok? */
					return ret;
				}
				break;
			case 'o':
				/* Here we don't have any way to know if we are in
				 * compression or decompression mode, since the argument
				 * order is out of our control. Thus, we have to perform
				 * this operation in any case, even through it may be
				 * useless under certain circumstances 
				 */
				oflag = 1;
				if ((ret = safe_strcpy(&output_file_name, optarg)) != 0){
					LOG(INFO, "Error parsing output filename"); /* TODO: it's ok? */
					return ret;
				}
				break;
			case '?':
				if (optopt == 'l' || optopt == 'i' || optopt == 'o')
					LOG(INFO, "Option -%c requires an argument. "
					"Try '-h' for more information", optopt);
				else if (isprint (optopt))
					LOG(INFO, "Unknown option '-%c'. "
					"Try '-h' for more information", optopt);				
				else
					LOG(INFO, "Unknown option character '\\x%x'"
					"Try '-h' for more information", optopt);					
				return 1;
			default:
				return -1;
		}
	}
	if (iflag == 0){
		LOG(INFO, "Missing input file. Try '-h' for more information");
		return 1;
	}
	if (dictionary_size == 0){
		dictionary_size = DICTIONARY_DEFAULT_SIZE;
	}
	
	uint8_t ofnflag = 0; //output_file_name_flag
	if ((oflag == 1) && (dflag == 0)){
		ofnflag = 1; 
	}
		LOG(INFO, "The following parameter have been choosen:\n"
			"Compress mode    = %s\n"
			"Dictionary size  = %d %s\n"
			"Verbose mode     = %s\n"
			"Input file name  = %s"
			"%s%s",
			 (dflag == 0)? "compression" : "decompression", 
			 dictionary_size, (lflag ==0)? "(default)" : "",
			 (__verbose == 1)? "on" : "off", input_file_name, 
			 (ofnflag == 1)? "\nOutput file name = " : "",
			 (ofnflag == 1)? output_file_name : "");
	 
	LOG(INFO, "starting...");
}
	


