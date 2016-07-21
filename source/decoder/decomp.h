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
#include <string.h>
#include <math.h>
#include "../common/bitio.h"
#include "../common/util.h"

typedef struct _code_t_ {
    char character;
    uint64_t parent_id;
} code_t;

const char *get_filename_ext(const char *filename);
int check_ext(const char *filename, const char *ext);

void decode(code_t *array, code_t node, struct bitio* b);
int decomp(const char *filename_enc, const char *filename_dec);
