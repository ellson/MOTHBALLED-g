#include <stdio.h>
#include <string.h>

# include "parse.h"

//unsigned char *test=(unsigned char*)"<a\\Aa 'bb' cc>";
//unsigned char *test=(unsigned char*)"<aa 'bb' cc>";
unsigned char *test=(unsigned char*)"<aa bb cc>";

int main (int argc, char *argv[]) {
    unsigned char *inp;
    int rc, i;

// FIXME - do options properly
    for (i=0; i<argc; i++) {
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

    inp = test;
    
    rc = parse(inp);

    return rc;
}
