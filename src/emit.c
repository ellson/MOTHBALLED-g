#include <stdio.h>

#include "emit.h"
#include "grammar.h"

#define OUT stdout
#define ERR stderr

static void print_string(unsigned char *frag, int flen) {
    int i;
    unsigned char *f;

    for (i=0, f=frag; i<flen; i++) {
        putc(*f++, OUT);
    }
}
static char char_prop(unsigned char prop, char noprop) {
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

void emit_start_state_machine(context_t *C) {
    putc(char_prop(0,'_'), OUT);
}

void emit_start_state(context_t *C, char *p) {
    fprintf(OUT,"%s ", NAMEP(p));
}

void emit_indent(context_t *C) {
    int i;

    putc('\n', OUT);
    for (i = C->nest*2; i--; ) putc (' ', OUT);
}

void emit_prop(context_t *C, unsigned char prop) {
    putc(char_prop(prop,'_'), OUT);
}

void emit_string(context_t *C, unsigned char *frag, int flen) {
    print_string(frag, flen);
}

void emit_token(context_t *C, unsigned char c) {
    putc(c, OUT);
}

void emit_end_state(context_t *C, int rc) {
    fprintf(OUT,"%d", rc);
}

void emit_end_state_machine(context_t *C) {
    putc('\n', OUT);
}

