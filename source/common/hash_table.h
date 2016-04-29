#include <stdint.h>

typedef struct _list_t_ {
    /*key*/
    char character;
    uint16_t parent_id;
    
    /*value*/
    uint16_t child_id;
    
    struct _list_t_ *next;
} list_t;

typedef struct _hash_table_t_ {
    int size;       /* the size of the table */
    list_t **table; /* the table elements */
} hash_table_t;

hash_table_t *create_hash_table(int);

unsigned int hash(hash_table_t*, char*);

list_t *lookup_string(hash_table_t*, char*);

int add_string(hash_table_t*, char*);

void free_table(hash_table_t*);