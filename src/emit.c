#include <stdio.h>
#include <stdlib.h>

#include "emit.h"
#include "grammar.h"

emit_t *emit;

void print_frag(unsigned char len, unsigned char *frag) {
    while (len--) putc(*frag++, OUT);
}

void print_string(unsigned char *len_frag) {
    unsigned char len;

    len = *len_frag++; 
    print_frag(len, len_frag); 
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
