#include <stdint.h>

struct bitio{
    FILE* f;
    uint64_t data;
    // --- writing index inside the buffer
    uint wp;
    // --- reading index inside the buffer
    uint rp; 
    uint mode;
}

int bit_write(struct* bitio, uint size, uint64_t data);
int bit_read(struct* bitio, uint max_size, uint64_t* result);

// --- This functions could do more than what their name suggest,
// --- e.g. close function could also perform a flush operation
// --- mode 0 --> read, mode 1 --> write (Rizzo's word)
struct bitio* bit_open(name, mode);
int bit_close(struct bitio*);

/*
 0 success
1,2,3... failure

0 success
-1 failure
errono settata
 */



