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
int comp(const struct gstate *state);

int fake_comp(const struct gstate* state, char* input_file, char* output_file);
