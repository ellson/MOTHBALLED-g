#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "list.h"
#include "emit.h"

static void api_sep(context_t *C) {
    putc (' ', OUT);
}

static void api_string(context_t *C, elem_t *string, int slen) {
    print_list(OUT, string);
}

static void api_tok(context_t *C, char class, unsigned char len, unsigned char *frag) {
    print_frag(OUT, len, frag);
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

static emit_t api1 = {
    /* api_start_state_machine */ NULL,
    /* api_sep */                 api_sep,
    /* api_start_state */         NULL,
    /* api_string */              api_string,
    /* api_frag */                NULL,
    /* api_tok */                 api_tok,
    /* api_end_state */           NULL,
    /* api_term */                NULL,
    /* api_end_state_machine */   api_end_state_machine,
    /* api_error */               api_error
};

static void api2_string(context_t *C, elem_t *string, int slen) {
    putc(' ', OUT);
    print_list(OUT, string);
}

static void api2_tok(context_t *C, char class, unsigned char len, unsigned char *frag) {
    putc('\n', OUT);
    print_frag(OUT, len, frag);
    putc(' ', OUT);
}

static emit_t api2 = {
    /* api_start_state_machine */ NULL,
    /* api_sep */                 NULL,
    /* api_start_state */         NULL,
    /* api_string */              api2_string,
    /* api_frag */                NULL,
    /* api_tok */                 api2_tok,
    /* api_end_state */           NULL,
    /* api_term */                api_term,
    /* api_end_state_machine */   api_end_state_machine,
    /* api_error */               api_error
};

emit_t *emit_g_api1 = &api1;
emit_t *emit_g_api2 = &api2;
