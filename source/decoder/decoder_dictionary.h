/* decoder_dictionary.h
 * 
 * Description----------------------------------------------------------
 * Declaration of the structure the will handles the decompression
 * process.
 * To manage the compressed file a (character,parent_id) table is
 * suggested.
 * ---------------------------------------------------------------------
 * 
 */
#include <stdint.h>

typedef struct{
	unsigned char character;
	uint16_t parent_id;
}node;

