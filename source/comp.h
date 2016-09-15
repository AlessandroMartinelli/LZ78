#include <sys/stat.h>		/* stat */
#include <unistd.h>			/* stat */
#include <math.h>			/* log, ceil */
#include <libgen.h>			/* for basename XPG version */
#include <inttypes.h>
#include "bitio.h"
#include "util.h"
#include "header.h"
#include "hash_table.h"

/* --- AVG_CODES_PER_ENTRY: spread factor 
 * used to increase collisions into the hash table
 */
#define AVG_CODES_PER_ENTRY 1

/* --- comp: compress the file using LZ78
 * Parameter:
 * - (input)state: the state of the compressor
 * Returns(int):
 * - 0: all fine
 * - -1: compression error
 */
int comp(const struct gstate *state);

/* --- fake_comp: compress the file using copy-paste after the compression 
 * header
 * Parameter:
 * - (input)state: the state of the compressor
 * - (input)input_file: name of the source file
 * - (input)output_file: name of the destination file
 * Returns(int):
 * - 0: all fine
 * - -1: compression error
 */
int fake_comp(const struct gstate* state);
