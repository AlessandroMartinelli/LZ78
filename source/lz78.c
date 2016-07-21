#include "util.h"

void usage(){
	printf("Usage:\t...");
}

int main(int ac, char**av){
	asdaf
	__verbose = 0;
	while((ch = getopt(ac,av, "cdvl:i:o:"))){
		case 'c': /* compress */
			break;
		case 'd': /* decompress */
			break;
		case 'v': /* verbose */
			__verbose = 1;
			break;
		case 'l': /* max dictionary length */
			break;
		case 'i': /* input file */
			break;
		case 'o': /* output file */
			break;
		default:
			usage();
	}
	
	return 0;
}
