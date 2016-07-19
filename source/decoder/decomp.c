/* decomp.c
 * 
 * Description----------------------------------------------------------
 * Definition of the behaviors of the decompressor.
 * Implementation of the methods declared in "decoder_dictionary.h".
 * ---------------------------------------------------------------------
 */

#include "decomp.h"

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

void decode(code_t *array, code_t node, struct bitio* b){
	if(node.parent_id!=0){
		decode(array, array[node.parent_id - 1], b);
	}
	bitio_write(b, sizeof(char)*8, node.character); /* emit character */
	return;
}

int decomp(const char *filename_enc, const char *filename_dec){
	struct bitio *b_enc, *b_dec;
	uint64_t aux;
	
	if(check_ext(filename_enc, "lz78") || check_ext(filename_dec, "txt")){
		errno = EINVAL;
		return -1;
	}
	
	b_enc = bitio_open(filename_enc, READ);
	b_dec = bitio_open(filename_dec, WRITE);
	if(b_enc == NULL || b_dec == NULL) return -1;
	
	fseek(b_enc->f, 0L, SEEK_END);
	uint size = ftell(b_enc->f);
	fseek(b_enc->f, 0L, SEEK_SET);
	size /= sizeof(code_t); /* normalize the number of array elements*/
	printf("size: %d\n",size);
	
	code_t nodes[size];
	
	for(int i=0; i<size; i++){
		if(bitio_read(b_enc, sizeof(code_t)*8, &aux) == sizeof(code_t)*8){
			memcpy(&nodes[i], &aux, sizeof(code_t));
			printf("<\"%c\", %d>\n", nodes[i].character, nodes[i].parent_id);
			decode(nodes, (code_t) nodes[i], b_dec);
		}
		else
			return -1;
	}
	
	bitio_close(b_enc);
	bitio_close(b_dec);
	return 0;
}
