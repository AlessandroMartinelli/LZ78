/* TODO: If the open return an error, how to set errno? in the paper, we have
 *  ENDFILE, but there is no such macros. There are ENFILE and EMFILE.
 * Alessandro: I propose renaming space --> "avail" and size --> "need"
 * Usually 0 indicates success, while -1 error. 
 */

#include "bitio.h"

int bitio_flush(struct bitio *b){
	if(b->mode == WRITE && b-> wp > 0){
	/* If e.g there is still 1 bit to write, it writes 1 byte; if there are
	*  9 bit to write, it writes 2 bytes. And so on.
	*/
		if((fwrite((void*)&b->data, (b->wp+7)/8, 1, b->f))!=1){ 
			errno = ENOSPC;
			return -1;
		}
	}
	return 0; // success
}

struct bitio* bitio_open(const char *name, mode_t mode){
	struct bitio *b;
	if (name == NULL || name[0] == '\0' || mode > WRITE){
		errno = EINVAL; 
		return NULL;
	}
	/* calloc: returns a properly aligned object */
	b = calloc(1, sizeof(struct bitio)); 
	if (b == NULL){
		errno = ENOMEM;
		return NULL;
	}
	
	/* fopen: if mode == 'w', it opens the file for writing only. 
	 *  If the file already exists, it is truncated to zero length. 
	 *  Otherwise a new file is created.
	 *  The stream is positioned at the beginning of the file.
	 */
	b->f = fopen(name, (mode == READ)? "r" : "w");
	if (b->f == NULL){
		errno = ENOENT;
		free(b);
		return NULL;
	}
	b->mode = mode;
	/* wp and rp were already setted to 0 by the calloc */
	return b;
}

int bitio_close(struct bitio* b){
	int ret = 0;
	if (b == NULL){
		errno = EINVAL;
		return -1;
	}
	ret = bitio_flush(b);

	fclose(b->f);	
	/* bzero: defensive programming: even through we are releasing the resources, 
	*  the caller could still try to access them
	*/
	bzero(b, sizeof(*b));	
	free(b);
	return ret;
}

int bitio_write(struct bitio *b, uint8_t size, uint64_t data){
	uint8_t space;
	if(b == NULL || b->mode !=WRITE || size > 64){
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
		/* the buffer fills, so we have to full the buffer, flush on file,
		*  and finally write the remaining bytes.
		*/
		b->data |= (data & ((1UL << space)-1)) << b->wp;
		if(fwrite(&(b->data), 8, 1, b->f)!=1){
			/* It is not possible to recover from a write error, therefore
			*  the caller must close the program in this case 
			*/
			errno = ENOSPC;
			return -1;
		}
		/* copy the remaining part of data in b->data and fill with 0 the first 
		*  part of b->data; finally, advance wp of the number of bit written
		*  this second time 
		*/
		data &= (1UL << size)-1;	
		b->data = data >> space; 
		b->wp = size - space;
	}
	return 0; // success!
}

int bitio_read(struct bitio* b, uint8_t size, uint64_t* data){
	uint8_t space; 
	if(b == NULL || b->mode != READ || size > 64){
		errno = EINVAL;
		return -1;
	}
	*data = 0;		   
	space = b->wp - b->rp; /* number of produced bytes still to be consumed */
	if(size == 0){
		return 0;
	}
	if (size <= space){
		/* There are enough bits to be consumed.
		*  We put in data "size" bits taken from [wp, rp], starting from rp 
		*/
		*data = ((b->data >> b->rp) & ((1UL << size)-1));
		b->rp+=size;
		return size;
	} else {
		/* The called would like to read more bytes than the buffer can offer,
		*  so we put into data all the available data; after that, since
		*  the buffer is now empty, we have to fill it again. We may read
		*  up to 8 items, each one 1 bytes long.
		*/
		*data = (b->data >> b->rp) & ((1UL << space)-1);
		int ret = fread(&(b->data), 1, 8, b->f);
		if (ret < 0){
			errno = ENODATA;
			return -1;
		}
		/* fread returns the number of object read, so in order to obtain
		*  the number of bits we have read, we have to multiply for the object
		*  size, which is 8 bits. 
		*/
		b->wp = ret * 8; 
		if(b->wp >= size-space){
			/* The number of bits extracted from the file is greater or 
			*  equal to the number of bit missing for satisfying the
			*  caller request. This means there are enought bits for
			*  satisfying the caller request.
			*/
			*data |= b->data << space;
			*data &= (1UL << size)-1;
			b->rp = size-space;
			return size;
		} else {
			/* Even extracting from the file, there were not enough bits for 
			*  satisfying the caller request
			*/
			*data |= b->data << space;
			*data &= (1UL<<(b->wp + space))-1;
			b->rp = b->wp;
			return b->wp + space;

		}
	}
}

FILE* bitio_get_file(struct bitio *b){
	if (b == NULL){
		errno = EINVAL;
		return NULL;
	}
	if(bitio_flush(b)==-1){
		/* TODO: errno has been setted by bitio_flush. 
		 * Should we set it again or not?
		 */
		return NULL;
	} 
	return b->f;
}







