/* outomp.c
 * 
 * Description----------------------------------------------------------
 * Definition of the behaviors of the outompressor.
 * Implementation of the methods outlared in "outoder_dictionary.h".
 * ---------------------------------------------------------------------
 */

#include "outomp.h"

const char *get_filename_ext(const char *filename) {
	const char *dot = strrchr(filename, '.');
	if(!dot || dot == filename) return "";
	return dot + 1;
}

int check_ext(const char *filename, const char *ext){
	if(!strcmp(get_filename_ext(filename),ext)){
		printf("%s matches!\n", filename);
		return 0;
	}
	return 1;
}

void decode(code_t *array, code_t node, struct bitio* b, uint8_t symbol_size){
	if(node.parent_id!=0){
		decode(array, array[node.parent_id - 1], b, symbol_size);
	}
	bitio_write(b, symbol_size, node.character); /* emit character */
	return;
}

int decomp(const char *filename_in, const char *filename_out){
	struct bitio *b_in, *b_out;
	char aux_char;
	uint size, num_of_codes;
	uint64_t aux_64;
	uint32_t aux_32;
	uint32_t dictionary_size;
	
	b_in = bitio_open(filename_in, READ);
	b_out = bitio_open(filename_out, WRITE);
	if(b_in == NULL || b_out == NULL) return -1;
	
	//( ---read the header of b_in
	bitio_read(b_in, sizeof(uint32_t)*8, aux_64);
	if(aux_64 != MAGIC) return -1; /* not an lz78 compressed file */
	
	bitio_read(b_in, sizeof(uint32_t)*8, dictionary_size);
	uint16_t id_size = (uint16_t) ceil(log(dictionary_size)); /* log base 2 */
	
	static uint8_t symbol_size = 8; /*bits (character)*/
	bitio_read(b_in, sizeof(uint32_t)*8, symbol_size);
	
	bitio_read(b_in, sizeof(uint64_t)*8, aux_64);
	if(strcmp(&aux_64, filename_out)!=0)
		/* not the initial name... and so... well... it's fine */;
	//---)
	
	/* get the number of codes */
	fseek(b_in->f, 0L, SEEK_END);
	num_of_codes = ftell(b_in->f);
	fseek(b_in->f, 0L, SEEK_SET);
	num_of_codes -= 3*32;                    /* neglete header size */
	num_of_codes /= symbol_size+id_size;     /* normalize the number of array
													      * elements by the code dimension*/
	/* the dictionary is clean every dictionary_size codes */
	size = (num_of_codes <= dictionary_size)? num_of_codes : dictionary_size;
	LOG(INFO,"size: %d\n",size);
	
	code_t nodes[size];
	
	for(int i=0; i<num_of_codes; i++){
		if(bitio_read(b_in, id_size, &aux_64) == id_size &&
			bitio_read(b_in, symbol_size, &aux_char) == symbol_size){
			nodes[i%size].character = (char) aux_char;
			nodes[i%size].parent_id = aux_64;
			LOG(INFO, "<\"%c\", %d>\n", nodes[i%size].character, nodes[i%size].parent_id);
			decode(nodes, nodes[i%size], b_out, symbol_size);
		}
		else{
			return -1
		}
		/*clean of the dictionary not needed*/
	}
	
	bitio_close(b_in);
	bitio_close(b_out);
	return 0;
}
