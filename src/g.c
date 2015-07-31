#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

# include "list.h"
# include "emit.h"
# include "parse.h"
# include "dumpg.h"

int main (int argc, char *argv[]) {
    int i, sz, space, opt;
    FILE *f;
    unsigned char *buf, *nextp;
#define BUFSZ 100000

    emit = emit_g_api1;

    while ((opt = getopt(argc, argv, "d:g:t")) != -1) {
        switch (opt) {
        case 'd':
            switch (atoi(optarg)) {
	    case 1: dumpg(); exit(0); break;
	    case 2: printg(); exit(0); break;
	    default:
		fprintf(stderr, "%s\n",
		    "-d1 = linear walk, -d2 = recursive walk"
		);
		exit(1);
		break;
	    }
            
            break;
        case 'g':
            switch (atoi(optarg)) {
	    case 1: emit = emit_g_api1; break;
	    case 2: emit = emit_g_api2; break;
	    default:
		fprintf(stderr, "%s\n",
		    "-g1 = minimal space, -g2 = shell-friendly spacing"
		);
		exit(1);
		break;
	    }
            break;
        case 't':
	    emit = emit_t_api;
	    break;
        default: /* '?' */
            fprintf(stderr,
		"Usage: %s [-d[12] | [-t] | [-g[12]] [ files ] [ - ]  \n",
		argv[0]
	    );
            exit(1);
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
