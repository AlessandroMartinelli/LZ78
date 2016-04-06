struct my_bitio{
    FILE* f;
    uint64_t data;
    uint wp; //indice all'interno del buffer per la scrittura
    uint rp; //indice all'interno del buffer per la lettura
    uint mode; //per mode andrà definito se ad esempio 0 vale read e 1 write
               //o il contrario
}

struct bitio& bit_open(const char* name, uint mode){
    struct bitio* b;
    if (name == NULL || name[0] == '\0' || mode > 1){
        errno = EINVAL; //da qualche parte va definita la variabile errno
        return NULL;
    }
    
}
