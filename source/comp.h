#include <sys/stat.h>		/* stat */
#include <unistd.h>			/* stat */
#include <math.h>			/* log, ceil */
#include <libgen.h>			/* for basename XPG version */
#include "bitio.h"
#include "util.h"
#include "header.h"
#include "hash_table.h"

#define AVG_CODES_PER_ENTRY 1 /* spread factor */

/* COMP
 * compressor main function
 */
int comp(char *filename_in, char *filename_out, uint32_t dictionary_size, uint8_t symbol_size);
