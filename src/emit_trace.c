#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "emit.h"

static void api_start_state_machine(context_t *C) {
    putc(char_prop(0,'_'), OUT);
}

static void api_start_state(context_t *C, char *p) {
    print_string(NAMEP(p));
}

static void api_sep(context_t *C) {
    putc (' ', OUT);
}

static void api_indent(context_t *C) {
    int i;

    putc('\n', OUT);
    for (i = C->nest*2; i--; ) putc (' ', OUT);
}

static void api_prop(context_t *C, unsigned char prop) {
    putc(char_prop(prop,'_'), OUT);
}

static void api_frag(context_t *C, unsigned char len, unsigned char *frag) {
    print_frag(OUT, len, frag);
}

static void api_end_state(context_t *C, int rc) {
    fprintf(OUT,"%d", rc);
}

static void api_term(context_t *C) {
    putc('\n', OUT);
}

static void api_end_state_machine(context_t *C) {
    putc('\n', OUT);
}

static void api_error(context_t *C, char *message) {
    fprintf(OUT, "\nError: %s\n", message);
    exit(1);
}


static emit_t api = {
    /* api_start_state_machine */ api_start_state_machine,
    /* api_sep */                 api_sep,
    /* api_indent */              api_indent,
    /* api_start_state */         api_start_state,
    /* api_prop */                api_prop,
    /* api_frag */                api_frag,
    /* api_end_state */           api_end_state,
    /* api_term */                api_term,
    /* api_end_state_machine */   api_end_state_machine,
    /* api_error */               api_error
};

emit_t *emit_trace_api = &api;
