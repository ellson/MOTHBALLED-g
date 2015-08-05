#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "list.h"
#include "inbuf.h"
#include "emit.h"

static void api_sep(context_t *C) {
    putc (' ', OUT);
}

static void api_string(context_t *C, elem_t *branch) {
    print_list(OUT, branch, -1, 0);
}

static void api_tok(context_t *C, char class, unsigned char len, unsigned char *frag) {
    print_frag(OUT, len, frag);
}

static void api_end_file(context_t *C) {
    putc('\n', OUT);
}

static void api_error(context_t *C, char *message) {
    fprintf(OUT, "\nError: %s\n", message);
    exit(1);
}

static emit_t api = {
    /* api_start_file */          NULL,
    /* api_sep */                 api_sep,
    /* api_start_state */         NULL,
    /* api_tree */                NULL,
    /* api_string */              api_string,
    /* api_frag */                NULL,
    /* api_tok */                 api_tok,
    /* api_end_state */           NULL,
    /* api_term */                NULL,
    /* api_end_file */            api_end_file,
    /* api_error */               api_error
};

static void api1_string(context_t *C, elem_t *branch) {
    print_list(OUT, branch, -1, 0);
}

static void api1_tok(context_t *C, char class, unsigned char len, unsigned char *frag) {
    putc('\n', OUT);
    print_frag(OUT, len, frag);
    putc(' ', OUT);
}

static emit_t api1 = {
    /* api_start_file */          NULL,
    /* api_sep */                 api_sep,
    /* api_start_state */         NULL,
    /* api_tree */                NULL,
    /* api_string */              api1_string,
    /* api_frag */                NULL,
    /* api_tok */                 api1_tok,
    /* api_end_state */           NULL,
    /* api_term */                NULL,
    /* api_end_file */            api_end_file,
    /* api_error */               api_error
};

emit_t *emit_g_api = &api;
emit_t *emit_g_api1 = &api1;
