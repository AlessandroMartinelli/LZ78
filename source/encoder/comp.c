#include "comp.h"

int comp(const char *filename_dec, const char *filename_enc){
	struct bitio *b_enc, *b_dec;
	uint16_t next_id = 1;
	uint16_t parent_id = 0;
	uint64_t result;
	char aux;
	list_t *node;
	
	printf("comp\n");
	
	if(check_ext(filename_enc, "lz78") || check_ext(filename_dec, "txt")){
		errno = EINVAL;
		return -1;
	}
	printf("comp1\n");
	
	b_enc = bitio_open(filename_enc, WRITE);
	b_dec = bitio_open(filename_dec, READ);
	if(b_enc == NULL || b_dec == NULL) return -1;
	
	hash_table_t *t;
	int size_of_table = 50; // ??
	t = create_hash_table(size_of_table);
	code_t code;
	
	int res = bitio_read(b_dec, sizeof(char)*8, &result);
	printf("comp: read %d bits (char=%c)\n",res, (char) result);
	while(res == sizeof(char)*8){
	/* untill characters are available*/
		node = lookup_code(t, (char) result, parent_id);
		if(node == NULL){ /* node not found */
		
			add_code(t, (char) result, parent_id, next_id++);
			code.character = (char) result;
			code.parent_id = parent_id;
			printf("emit code #%d: <\"%c\", %d>\n", next_id-1, (char) result, parent_id);
			
			bzero(&result, sizeof(uint64_t));
			memcpy(&result, &code, sizeof(code_t));
			bitio_write(b_enc, sizeof(code_t)*8, result); /* emit code */
			
			parent_id = 0;
		}
		else{
			printf("comp: code found\n");
			parent_id = node->child_id;
		}
		bzero(&result, sizeof(uint64_t));
		res = bitio_read(b_dec, sizeof(char)*8, &result);
		printf("comp: read %d bits (char=%c)\n",res, (char) result);
	}
	/* just to be sure it will emits all the characters? */
	if(node != NULL){
		code.character = node->character;
		code.parent_id = node->parent_id;
		printf("codee: <\"%c\", %d>\n", node->character, node->parent_id);
		bzero(&result, sizeof(uint64_t));
		memcpy(&result, &code, sizeof(code_t));
		bitio_write(b_enc, sizeof(code_t)*8, result); /* emit code */
	}
	
	free_table(t);
	
	bitio_close(b_enc);
	bitio_close(b_dec);
	return 0;
}
