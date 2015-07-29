#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "emit.h"

#if 0
static void api_start_state_machine(context_t *C) {
    putc(char_prop(0,'_'), OUT);
}
#endif

static void api_start_state(context_t *C, char *p) {
    // FIXME - weird C bug
    //       - without this empty func the parser behaves differemtly!
}

#if 0
static void api_indent(context_t *C) {
    int i;

    putc('\n', OUT);
    for (i = C->nest*2; i--; ) putc (' ', OUT);
}

static void api_prop(context_t *C, unsigned char prop) {
    putc(char_prop(prop,'_'), OUT);
}
#endif

static void api_frag(context_t *C, unsigned char len, unsigned char *frag) {
    print_frag(len, frag);
    putc(' ', OUT);
}

static void api_token(context_t *C, unsigned char c) {
    putc(c, OUT);
    putc(' ', OUT);
}

#if 0
static void api_end_state(context_t *C, int rc) {
    fprintf(OUT,"%d", rc);
}

static void api_term(context_t *C) {
    putc('\n', OUT);
}
#endif

static void api_end_state_machine(context_t *C) {
    putc('\n', OUT);
}

static void api_error(context_t *C, char *message) {
    fprintf(OUT, "\nError: %s\n", message);
    exit(1);
}


static emit_t api = {
    /* api_start_state_machine */ NULL,
    /* api_indent */              NULL,
    /* api_start_state */         api_start_state,
    /* api_prop */                NULL,
    /* api_frag */                api_frag,
    /* api_token */               api_token,
    /* api_end_state */           NULL,
    /* api_term */                NULL,
    /* api_end_state_machine */   api_end_state_machine,
    /* api_error */               api_error
};

emit_t *emit_g_api = &api;
