#include <sys/stat.h>		/* stat */
#include <unistd.h>			/* stat */
#include <math.h>			/* log, ceil */
#include <libgen.h>			/* for basename XPG version */
#include "bitio.h"
#include "util.h"
#include "header.h"
#include "hash_table.h"

/* PATH_TO_LZ78NAME
 *  Convert a filename, possibly comprehensive of path, 
 *  to the base name of the file, with extension changed to .lz78.
 *  As of now, it only works with name of length lesser than 256.
 */
char* path_to_lz78name(char* path);

/* COMP
 * compressor main function
 */
int comp(char *filename_in, char *filename_out, uint32_t dictionary_size, uint8_t symbol_size);
