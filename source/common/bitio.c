/* TODO: define the errno variabile;
*  If the open return an error, how to set errno? in the paper, we have
*  ENDFILE, but there is no such macros. There are ENFILE and EMFILE.
*  Alessandro: propongo di rinominare space in "avail" e size in "need"
*/

#include "bitio.h"

// ------------------ //
//     BITIO_OPEN     //
// ------------------ //
struct bitio* bitio_open(const char* name, uint mode){
    struct bitio* b;
    if (name == NULL || name[0] == '\0' || mode > 1){
        errno = EINVAL; 
        return NULL;
    }
    b = calloc(1, sizeof(struct bitio)); /* returns a properly aligned object */
    if (b == NULL){
        errno = ENOMEM;
        return NULL;
    }
    
	/* fopen: if mode == 'w', it opens the file for writing only. 
	*  If the file already exists, it is truncated to zero length. 
    *  Otherwise a new file is created.
	*  The stream is positioned at the beginning of the file.
	*/
    b->f = fopen(name, (mode == 0)? "r" : "w");
    if (b->f == NULL){
        errno = ENFILE;
        free(b);
        return NULL;
    }
    b->mode = mode;
    /* wp and rp were already setted to 0 by the calloc */
    return b;
}

// ------------------- //
//     BITIO_CLOSE     //
// ------------------- //
int bitio_close(struct bitio* b){
    int ret = 0;
    if (b == NULL){
        errno = EINVAL;
        return -1;
    }
    /* TODO: the return value is not so simple (see man) */
    if(b->mode == 1 && b-> wp > 0){
        if(fwrite((void*)&b->data,(b->wp+7)/8, 1, b->f)){ //TODO: the "1" was added by Ale in order to have the program to compile
            ret = -1;
        }
    }
    fclose(b->f);
	/* bzero: defensive programming: even through we are releasing the resources, 
	*  the caller could still try to access them
	*/
	bzero(b, sizeof(*b));	
    free(b);
    return ret;
}

// ------------------- //
//     BITIO_WRITE     //
// ------------------- //
int bitio_write(struct bitio* b, uint size, uint64_t data){
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
        data &= (1UL << size)-1;	/* clear the higher part of the data */
        b->data |= data << b->wp;	/* copy the data to the buffer */
        b->wp += size;
    } else {
		/* the buffer fills, so we have to full the buffer, flush,
		*  and then rewrite the remaining bytes.
		*/
		data &= (1UL << size)-1;	
        b->data |= data << b->wp;
        if(fwrite(&(b->data), 1, 8, b->f)!=1){ // TODO: check: 8 itemes of 1 bytes?
			/* It is not possible to recover from a write error, therefore
			*  the caller must close the program in this case 
			*/
            errno = ENOSPC;
            return -1;
        }
        b->data = data >> space; /* unsigned integer shift --> fill with zeroes */
        b->wp = size - space;
    }
    return 0; // success!
}

// ------------------ //
//     BITIO_READ     //
// ------------------ //
int bitio_read(struct bitio* b, uint size, uint64_t* data){
    int space; 
    if(b == NULL || b->mode != 0 || size > 64){
        errno = EINVAL;
        return -1;
    }
    *data = 0;           
    space = b->wp - b->rp; /* number of produced bytes still to be consumed */
    if(size == 0){
		return 0;
    }
    if (size <= space){
    	/* There is enough space for storing the "size" bytes passed as argument.
    	*  We put in *data "size" bytes taken from [wp, rp], starting from rp 
    	*/
        *data = ((b->data >> b->rp) & (1UL << size)-1);
        b->rp+=size;
    } else {
        /* The called would like to read more bytes than the buffer can offer,
        *  so we put into "data" all the available data; after that, since
        *  the buffer is now empty, we have to fill it again. We may read
        *  up to 8 items, each one 1 bytes long.
        */
        *data = (b->data >> b->rp);
        uint ret = fread(&(b->data), 1, 8, b->f); //TODO: I'm sure there is a better way for &(b->data) 
    	if (ret < 0){
        	errno = ENODATA;
        	return -1;
    	}
		b->wp = ret * 8; /* fread returns the number of object read*/
		if(b->wp >= size-space){
			/* I have to read in the data also the remaining ones
			*  once reloaded from the file
			*/
		    *data |= b->data << space;
		    *data &= (1UL << size)-1;
		    b -> rp = size-space;
		    return size;
		} else {
			/* If the file is going to end, then size bytes cannot be read */
		    *data |= b->data << space;
		    *data &= 1UL<<(b->wp + space)-1;
		    b->rp = b->wp;
		    return b->wp + space;
		}
	}
}










