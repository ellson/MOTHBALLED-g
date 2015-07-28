#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# include "parse.h"

int main (int argc, char *argv[]) {
    int rc, i;
    unsigned char *buf;
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

// FIXME - accept file argument(s)
// FIXME - do proper buffer management
    buf = malloc(BUFSZ);
    fread(buf, 1, BUFSZ, stdin);

    rc = parse(buf);

    return rc;
}
