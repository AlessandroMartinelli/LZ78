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
#include "../common/bitio.h"

typedef struct _code_t_ {
    char character;
    uint8_t parent_id;
    /* the last member is padded with the number of bytes required so that the
     * total size of the structure should be a multiple of the largest alignment
     * of any structure member:
     * -) uint16_t -> sizeof(code_t)=4	(aligned to 2 bytes)
     * -) uint8_t -> sizeof(code_t)=2	(aligned to 1 byte)
     */
} code_t;

const char *get_filename_ext(const char *filename);
int check_ext(const char *filename, const char *ext);

void decode(code_t *array, code_t node, struct bitio* b);
int decomp(const char *filename_enc, const char *filename_dec);
