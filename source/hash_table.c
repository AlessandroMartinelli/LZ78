#include "hash_table.h"

hash_table_t *create_hash_table(uint32_t size) {
	hash_table_t *new_table;
	uint32_t i;

	if (size < 1){ /* invalid size for table */
		errno = EINVAL;
		return NULL;
	}

	/* Attempt to allocate memory for the table structure */
	if ((new_table = malloc(sizeof(hash_table_t))) == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	/* Attempt to allocate memory for the table itself */
	if ((new_table->table = malloc(sizeof(list_t *) * size)) == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	/* Initialize the elements of the table */
	for (i = 0; i < size; i++) new_table->table[i] = NULL;

	/* Set the table's size */
	new_table->size = size;

	return new_table;
}

/* simply way to implement a hash function */
uint32_t hash(hash_table_t *hashtable, char c, uint32_t id) {
	uint32_t hashkey;

	/* we start our hash out concateneting 4 times c */
	hashkey = c + (c << 8) + (c << 16) + (c << 24);

	/* Do the XOR bit-by-bit between hashkey and id */
	hashkey ^= id;

	/* we then return the hash value mod the hashtable size so that it will
	 * fit into the necessary range
	 */
	return hashkey % hashtable->size;
}

list_t *lookup_code(hash_table_t *hashtable, char c, uint32_t id) {
	list_t *list;
	uint32_t hashkey = hash(hashtable, c, id);

	/* Go to the correct list based on the hash value and see if the node is
	 * in the list.  If it is, return a pointer to the list element.
	 * If it isn't, the item isn't in the table, so return NULL.
	 */
	for (list = hashtable->table[hashkey]; list != NULL; list = list->next) {
		if (c == list->character && id == list->parent_id) return list;
	}
	return NULL;
}

int add_code(hash_table_t *hashtable, char c, uint32_t id, uint32_t new_node) {
	list_t *new_list;
	list_t *current_list;
	uint32_t hashkey = hash(hashtable, c, id);

	/* Attempt to allocate memory for list */
	if ((new_list = malloc(sizeof (list_t))) == NULL) return 1;

	/* Does item already exist? */
	current_list = lookup_code(hashtable, c, id);
	/* item already exists, don't insert it again. */
	if (current_list != NULL){
		free(new_list);
		return -1;
	}
	
	/* Add new node */
	new_list->character = c;
	new_list->parent_id = id;
	new_list->child_id = new_node;
	/* Insert into list */
	new_list->next = hashtable->table[hashkey];
	hashtable->table[hashkey] = new_list;

	return 0;
}

void free_table(hash_table_t *hashtable){
	uint32_t i;
	list_t *list, *temp;

	if (hashtable==NULL) return;

	/* Free the memory for every item in the table, including the 
	 * strings themselves.
	 */
	for(i=0; i<hashtable->size; i++) {
		list = hashtable->table[i];
		while(list!=NULL) {
			temp = list;
			list = list->next;
			free(temp);
		}
	}

	/* Free the table itself */
	free(hashtable->table);
	free(hashtable);
}
