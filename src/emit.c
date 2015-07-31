#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "list.h"
#include "emit.h"

emit_t *emit;

void print_frag(FILE *chan, unsigned char len, unsigned char *frag) {
    while (len--) putc(*frag++, chan);
}

void print_string(unsigned char *len_frag) {
    unsigned char len;

    len = *len_frag++; 
    print_frag(OUT, len, len_frag); 
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
