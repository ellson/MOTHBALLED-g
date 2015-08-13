#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"
#include "stats.h"

emit_t *emit;

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

void print_error(context_t *C, state_t si, char *message) {
    unsigned char *p, c;
    long lineno;
    
    lineno = stat_lfcount?stat_lfcount:stat_crcount;

    fprintf(C->err, "\nError: %s ", message);
    print_len_frag(C->err, NAMEP(si));
    fprintf(C->err, " on line: %ld just before: \"", lineno);
    p = C->in;
    while ((c = *p++)) {
        if (c == '\n' || c == '\r') break;
        putc(c, C->err);
    }
    fprintf(C->err, "\"\n");
    exit(1);
}
