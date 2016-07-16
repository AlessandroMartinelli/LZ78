/* TODO: substitute uint with the appropriate type.
*  For example, mode can be represented with an uint8_t, since it only
*  assume 2 values, 0 or 1.
*  In bitio_write, we check the size must not be greater than 64; thus,
*  we may use a uint8_t for that.
*/

#include <stdint.h> // for uintXX_t
#include <stdlib.h>	// for calloc
#include <stdio.h>  // for dealing with file: FILE, fwrite etc
#include <errno.h>  // for using the variabile errno
#include <string.h> // bzero

struct bitio{
    FILE* f;
    uint64_t data;
    uint wp; 	/* writing index inside the buffer */
    uint rp; 	/* reading index inside the buffer */
    uint mode;  /* 0 means reading, 1 means writing */
};

int bitio_write(struct bitio*, uint size, uint64_t data);
int bitio_read(struct bitio*, uint max_size, uint64_t* result);
struct bitio* bitio_open(const char* name, uint mode);
int bitio_close(struct bitio*);
