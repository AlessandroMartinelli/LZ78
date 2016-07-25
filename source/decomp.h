/* decomp.h
 * 
 * Description----------------------------------------------------------
 * Declaration of the structure the will handles the decompression
 * process.
 * To manage the compressed file a (character,parent_id) table is
 * suggested.
 * ---------------------------------------------------------------------
 * 
 */
#include <sys/stat.h>		/* stat */
#include <unistd.h>			/* stat */
#include <string.h>
#include <math.h>
#include "bitio.h"
#include "util.h"
#include "header.h"

typedef struct _code_t_ {
    char character;
    uint64_t parent_id;
} code_t;

void decode(code_t *array, code_t node, struct bitio* b, uint8_t symbol_size);
int decomp(const char *filename_enc, const char *filename_dec);
