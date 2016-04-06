struct my_bitio;

int bitio_write(struct *bitio, uint size, uint64_t data);
int bitio_read(struct *bitio, uint max_size, uint64_t* result);

//Questa funzioni potrebbero fare qualcosa di più di quello che dice il titolo,
//ad esempio la close potrebbe fare anche la flush.
struct bitio* bitio_open(name, mode);
int bitio_close(struct bitio*);


0 success
1,2,3... failure

0 success
-1 failure
errono settata


