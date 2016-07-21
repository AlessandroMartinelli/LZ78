/* TODO: substitute uint with the appropriate type.
*  For example, mode can be represented with an uint8_t, since it only
*  assume 2 values, 0 or 1.
*  In bitio_write, we check the size must not be greater than 64; thus,
*  we may use a uint8_t for size.
*/

#include <stdint.h> // for uintXX_t
#include <stdlib.h>	// for calloc
#include <stdio.h>  // for dealing with file: FILE, fwrite etc
#include <errno.h>  // for using the variabile errno
#include <string.h> // bzero

enum mode_t {READ, WRITE};

struct bitio{
    FILE* f;
    uint64_t data;
    uint8_t wp; 	/* writing index inside the buffer */
    uint8_t rp; 	/* reading index inside the buffer */
    mode_t mode;
};

int bitio_write(struct bitio*, uint8_t size, uint64_t data);
int bitio_read(struct bitio*, uint8_t max_size, uint64_t* result);
struct bitio* bitio_open(const char* filename, mode_t mode);
int bitio_close(struct bitio*);
FILE* bitio_get_file(struct bitio*);
