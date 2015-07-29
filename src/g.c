#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

# include "emit.h"
# include "parse.h"
# include "dumpg.h"

int main (int argc, char *argv[]) {
    int i, sz, space, opt;
    FILE *f;
    unsigned char *buf, *nextp;
#define BUFSZ 100000

    emit = emit_g_api;

    while ((opt = getopt(argc, argv, "dgt")) != -1) {
        switch (opt) {
        case 'd':
            dumpg();
	    exit (0);
            break;
        case 'g':
            printg();
	    exit (0);
            break;
        case 't':
	    emit = emit_trace_api;
	    break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [ -d | -g | [ files ] [ - ]  \n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

// FIXME - do proper buffer management
    buf = malloc(BUFSZ);

    nextp = buf;
    space = BUFSZ;
    for (i=optind; i<argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            fread(nextp, 1, space, stdin);
	    break;
        }
        else {
            f = fopen(argv[i],"r");
	    if (f) {
	        sz = fread(nextp, 1, space, f);
                space -= sz;
                nextp += sz;
                fclose(f);
	    }
	    else {
		fprintf(stderr, "file \"%s\" is not readable\n", argv[i]);
		exit(1);
	    }
        }
    }

    parse(buf);

    return 0;
}
