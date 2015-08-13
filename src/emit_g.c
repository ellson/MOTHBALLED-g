#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

static void api_sep(context_t *C) {
    putc (' ', OUT);
}

static void api_string(context_t *C, elem_t *branch) {
    print_list(OUT, branch, -1, 0);
}

static void api_token(context_t *C, char token) {
    putc(token, OUT);
}

static void api_end_activity(context_t *C) {
    putc('\n', OUT);
}

static emit_t api = {
    /* api_start_file */          NULL,
    /* api_start_activity */      NULL,
    /* api_sep */                 api_sep,
    /* api_start_state */         NULL,
    /* api_act */                 NULL,
    /* api_subject */             NULL,
    /* api_string */              api_string,
    /* api_frag */                NULL,
    /* api_token */               api_token,
    /* api_end_state */           NULL,
    /* api_term */                NULL,
    /* api_end_activity */        api_end_activity,
    /* api_end_file */            NULL,
    /* api_error */               print_error
};

static void api1_string(context_t *C, elem_t *branch) {
    print_list(OUT, branch, -1, 0);
}

static void api1_token(context_t *C, char token) {
    fprintf(OUT,"\n%c ", token);
}

static void api1_term(context_t *C) {
    putc('\n', OUT);
}

static emit_t api1 = {
    /* api_start_file */          NULL,
    /* api_start_activity */      NULL,
    /* api_sep */                 api_sep,
    /* api_start_state */         NULL,
    /* api_act */                 NULL,
    /* api_subject */             NULL,
    /* api_string */              api1_string,
    /* api_frag */                NULL,
    /* api_token */               api1_token,
    /* api_end_state */           NULL,
    /* api_term */                api1_term,
    /* api_end_activity */        api_end_activity,
    /* api_end_file */            NULL,
    /* api_error */               print_error
};

emit_t *emit_g_api = &api;
emit_t *emit_g_api1 = &api1;
