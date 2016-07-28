//#include <ctype.h>		// for isprint
#include <stdlib.h>			// for strtol
#include <unistd.h>			// for getopt, access
#include <stdint.h>			// for uintXX_t
#include <errno.h>			// for using the variabile errno
//#include <libgen.h>			// for basename XPG version
#include "util.h"
#include "comp.h"
#include "decomp.h"

/* PATH_TO_LZ78NAME
 *  Convert a filename, possibly comprehensive of path, 
 *  to the base name of the file, with extension changed to .lz78.
 *  As of now, it only works with name of length lesser than 256.
 */
char* path_to_lz78name(char* path);

int comp_init_gstate(struct gstate* state, char* input_file, char* output_file, uint32_t dictionary_len);

int decomp_init_gstate(struct gstate* state, char* input_file, char* output_file, uint64_t *f_dim);

int comp_chooser(struct gstate* state);

int decomp_chooser( );

void usage();
