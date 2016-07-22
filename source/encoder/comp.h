#include <sys/stat.h>		/* stat */
#include <unistd.h>			/* stat */
#include <openssl/md5.h>	/* checksum */
#include <math.h>				/* log, ceil */
#include "../common/bitio.h"
#include "../common/util.h"
#include "hash_table.h"

int comp(const char *filename_dec, const char *filename_enc);
