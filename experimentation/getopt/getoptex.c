#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
int
main(int argc, char *argv[])
{
	int flags, opt;
	int n, t, tfnd;
	
	n = 0; t = 0;
	tfnd = 0;
	flags = 0;
	while ((opt = getopt(argc, argv, "n::t:")) != -1) {
		switch (opt) {
			case 'n':
				if(optarg != NULL) n = atoi(optarg);
				flags = 1;
				break;
			case 't':
				t = atoi(optarg);
				tfnd = 1;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	
	printf("flags=%d; tfnd=%d; optind=%d;\nn=%d; t=%d\n", flags, tfnd, optind, n, t);
	
	if (optind >= argc) {
		fprintf(stderr, "Expected argument after options\n");
		exit(EXIT_FAILURE);
	}
	
	printf("name argument = %s\n", argv[optind]);
	
	/* Other code omitted */
	
	exit(EXIT_SUCCESS);
}
