#include <sys/stat.h>		/* stat */
#include <unistd.h>			/* stat */
#include <math.h>				/* log, ceil */
//#include "../common/bitio.h"
//#include "../common/util.h"
//#include "../common/header.h"
#include "hash_table.h"

int comp(char *filename_in, char *filename_out, uint32_t dictionary_size, uint8_t symbol_size);
