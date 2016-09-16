#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <openssl/md5.h>	//checksum

typedef struct _list_t_ {
    /*key: <parent node id, character>*/
    char character;
    uint32_t parent_id;
    
    /*value: <child node id>*/
    uint32_t child_id;
    
    struct _list_t_ *next;
} list_t;

typedef struct _hash_table_t_ {
    uint32_t size;       /* the size of the table */
    list_t **table; /* the table elements */
} hash_table_t;

/* --- create_hash_table: create the hash table and initialize all the elements
 * Parameter:
 * - (input)uint32_t: the size of the table
 * Returns(pointer to a struct hast_table_t)
 * - NULL:  error in allocating the new node memory space or
 *   error in allocating the table memory space
 */
hash_table_t *create_hash_table(uint32_t);

/* --- hash: hash function
 * Parameters
 * - (input)pointer to a struct hast_table_t: used to establish the size of 
 *   the table
 * - (input)char: the character of the code to hash
 * - (input)unit32_t: the parent node id
 * Return(uint32_t): hashed the key <parent id, character>
 */
uint32_t hash(hash_table_t*, char, uint32_t);

/* --- lookup_code: find a pair <parent id, char> in the hash table
 * Parameters:
 * - (input)pointer to a struct hast_table_t: used to establish the size of 
 *   the table
 * - (input)char: the character of the code to hash
 * - (input)unit32_t: the parent node id
 * Returns(list_t): the head of the list of <child nodes id> belong the passed key
 *   domain
 * - NULL: node not found
 */
list_t *lookup_code(hash_table_t*, char, uint32_t);

/* --- add_code: add a <node> with key <parent id, char> in the hash table
 * Parameters:
 * - (input)pointer to a struct hast_table_t: used to establish the size of 
 *   the table
 * - (input)char: the character of the code to hash
 * - (input)unit32_t: the parent node id
 * - (input)uint32_t: the child node id (the new node)
 * Returns(int):
 * - 0: all fine
 * - -1: node already in the hash table
 * - 1: error in allocating the new node memory space
 */
int add_code(hash_table_t*, char, uint32_t, uint32_t);

/* --- free_table: clean the hash table
 * Paramenters:
 * - (input)pointer to a struct hast_table_t: used to establish the size of 
 *   the table
 */
void free_table(hash_table_t*);
