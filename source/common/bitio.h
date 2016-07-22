/* TODO: substitute uint with the appropriate type.
*  For example, mode can be represented with an uint8_t, since it only
*  assume 2 values, 0 or 1.
*  In bitio_write, we check the size must not be greater than 64; thus,
*  we may use a uint8_t for size.
*/

#ifndef _BITIO_H
#define _BITIO_H

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

/* BITIO_FLUSH
 *  If the file was opened in write mode, it perform a flush of
 *  the bitio buffer on the file related to the bitio structure.
 */
int bitio_flush(struct bitio*);

/* BITIO_OPEN
 *  It tries to allocate space for a struct bitio, then open a file and
 *  assigns its descriptor to the FILE pointer within struct bitio.
 *  After, it sets the "mode" value inside struct bitio accordingly to the
 *  argument passed by the caller. Finally, it returns the created struct bitio
 */
struct bitio* bitio_open(const char* filename, mode_t mode);

/* BITIO_CLOSE
*  If in write mode, it flushes the buffer content on file. 
*  Finally, it closes the file, deallocate the struct bitio and set to
*  zero the corresponding memory.
*/
int bitio_close(struct bitio*);

/* BITIO_WRITE
*  Copy "size" (up to 64) bit from data to b->data. If there is not enough space,
*  write as much as you can, flush the buffer to b->f and then
*  write the remaining part.
*/
int bitio_write(struct bitio*, uint8_t size, uint64_t data);

/* BITIO_READ
*  It copies up to size bits from struct_bitio->data into data.
*  If struct_bitio->data doesn't contain enough data, it tries to
*  read some bits from the file referred to by struct_bitio->f
*/
int bitio_read(struct bitio*, uint8_t max_size, uint64_t* result);

/* BITIO_GET_FILE
 *  This function is used when you want to write to a file
 *  already opened with bitio_open without causing inconsistences.
 *  It flush the bitio_write buffer to file, and then returns 
 *  the pointer to the file, positioned where bitio_write left it.
 */
FILE* bitio_get_file(struct bitio*);

#endif
