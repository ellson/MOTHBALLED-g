#include <stdio.h>
#include <stdlib.h>

#include "emit.h"
#include "grammar.h"

emit_t *emit;

void print_string(unsigned char *frag, int flen) {
    int i;
    unsigned char *f;

    for (i=0, f=frag; i<flen; i++) {
        putc(*f++, OUT);
    }
}

char char_prop(unsigned char prop, char noprop) {
    char c;

    if (prop & ALT) { c = '|'; }
    else {
        if (prop & OPT) {
            if (prop & (SREP|REP)) { c = '*'; }
            else { c = '?'; }
        }
        else {
            if (prop & (SREP|REP)) { c = '+'; }
            else { c = noprop; }
        }
    }
    return c;
}

