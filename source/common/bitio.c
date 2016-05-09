#include "bitio.h"

// ---------------- //
//     BIT_OPEN     //
// ---------------- //
struct bitio* bit_open(const char* name, uint mode){
    struct bitio* b;
    if (name == NULL || name[0] == '\0' || mode > 1){
        // --- TODO: we have to define errno variabile
        errno = EINVAL; 
        return NULL;
    }
    b = calloc(1, sizeof(struct bitio));
    
    if (b == NULL){
        errono = ENOMEM;
        return NULL;
    }
    
    // --- w: Truncate file to zero length or create text file for writing. The stream is positioned at the beginning of the file.
    b->f = fopen(name, (mode == 0)? "r" : "w");
    if (b->f == NULL){
        errno = ENDFILE;
        free(b);
        return NULL;
    }
    b->mode = mode;
    // --- wp and rp already 0
    return b;
}

// ----------------- //
//     BIT_CLOSE     //
// ----------------- //
int bit_close(Struct bitio* b){
    int ret = 0;
    if (b == NULL){
        errno = EINVAL;
        return -1;
    }
    // --- Here lies a strange statement...
    if(b->mode == 1 && b-> wp > 0){
        if(fwrite((void*)&b->data,(b->wp+7)/8, b->f)){
            ret = -1;
        }
    }
    fclose(b->f);
    free(b);
    return ret;
}

// ----------------- //
//     BIT_WRITE     //
// ----------------- //
int bit_write(struct bitio* b, uint size, uint64_t data){
    int space;
    if(b == NULL || b->mode !=1 || size > 64){
        errno = EINVAL;
        return -1;
    }
    if (size == 0){
        return 0;
    }
    space = 64 - b->wp;
    if (size <= space){
        b->data& = (1 << size) -1;
        b->wp += size;
    } else {
        b-> data |= data << b->wp;
        // --- Where was a function Gianni Pollina said had argument exchanged. Maybe is this? (8 and 1. See man)
        // --- It is not possible to recover from a write error, therefore the caller must close the program in this case.
        if(fwrite(b->data, 1, 8, b->f))=1{
            errno = ENDSPACE;
            return -1;
        }
        // --- A shift of unsigned integer fill with zeroes, both if it's a left shift or it it's a right shift
        b->data = data >> space; 
        b->wp = size - space;
        // --- Success!
        return 0; 
    }
}

// ---------------- //
//     BIT_READ     //
// ---------------- //
int bit_read(struct bitio* b, uint size, uint64_t* data){
    // --- Space is the size of the buffer, i.e. how many byte I can read at most
    int space;
    if(b == NULL || b->mode != 0 || size > 64){
        errno = EINVAL;
        return -1;
    }
    *data = 0;           
    space = b->wp - b->rp;
    if(size == 0){
        // --- TODO: qui non ho scritto io o cosa?
    }
    if (size <= space){
        // --- TODO: Check the following line
        *data = ((b->data >> b->rp)&(1 UL << size)-1);
        b->rp+=size;
    } else {
        // --- The called would like to read more bytes than the buffer can offer
        *data = (b->data >> b->rp);
        ret = fread(b->data, 1, 8, b);
    }
    if (ret < 0){
        errno = ENODATA;
        return -1;
    }
    b->wp = ret * 8;
    if(b->wp >= size-space){
        *data |= b->data << space;
        *data &= (NULL << size) -1;
        b -> rp = size-space;
        return size;
    } else {
        *data |= b->data << space;
        *data &= NULL<<(b->wp + space)-1;
        b->rp = b->wp;
        return b->wp + space;
    }
    
}

