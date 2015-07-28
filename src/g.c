#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# include "parse.h"

int main (int argc, char *argv[]) {
    int rc, i, sz, space;
    FILE *f;
    unsigned char *buf, *nextp;
#define BUFSZ 100000

// FIXME - do options properly
    for (i=1; i<argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
	    set_sstyle();
        }
        if (strcmp(argv[i], "-g") == 0) {
            printg();
	    return 0;
        }
        if (strcmp(argv[i], "-d") == 0) {
            dumpg();
	    return 0;
        }
    }

// FIXME - do proper buffer management
    buf = malloc(BUFSZ);

    nextp = buf;
    space = BUFSZ;
    for (i=1; i<argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            fread(nextp, 1, space, stdin);
	    break;
        }
        else {
            f = fopen(argv[i],"r");
	    sz = fread(nextp, 1, space, f);
	    fclose(f);
            space -= sz;
            nextp += sz;
        }
    }

    rc = parse(buf);

    return rc;
}
