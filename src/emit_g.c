#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

static void api_sep(context_t *C) {
    putc (' ', C->out);
}

static void api_string(context_t *C, elem_t *branch) {
    print_list(C->out, branch, -1, 0);
}

static void api_token(context_t *C, char token) {
    putc(token, C->out);
}

static void api_end_parse(context_t *C) {
    putc('\n', C->out);
}

static emit_t api = {
    /* api_start_parse */         NULL,
    /* api_end_parse */           api_end_parse,

    /* api_start_file */          NULL,
    /* api_end_file */            NULL,

    /* api_start_activity */      NULL,
    /* api_end_activity */        NULL,

    /* api_start_act */           NULL,
    /* api_end_act */             NULL,

    /* api_start_subject */       NULL,
    /* api_end_subject */         NULL,

    /* api_start_attributes */    NULL,
    /* api_end_attributes */      NULL,

    /* api_start_container */     NULL,
    /* api_end_container */       NULL,

    /* api_start_state */         NULL,
    /* api_end_state */           NULL,

    /* api_act */                 NULL,
    /* api_subject */             NULL,
    /* api_attributes */          NULL,

    /* api_sep */                 api_sep,
    /* api_token */               api_token,
    /* api_string */              api_string,

    /* api_frag */                NULL,

    /* api_error */               print_error
};


static void api1_token(context_t *C, char token) {
    putc ('\n', C->out);
    putc(token, C->out);
    putc (' ', C->out);
}

static void api1_end_container(context_t *C) {
    putc('\n', C->out);
}

static emit_t api1 = {
    /* api_start_parse */         NULL,
    /* api_end_parse */           api_end_parse,

    /* api_start_file */          NULL,
    /* api_end_file */            NULL,

    /* api_start_activity */      NULL,
    /* api_end_activity */        NULL,

    /* api_start_act */           NULL,
    /* api_end_act */             NULL,

    /* api_start_subject */       NULL,
    /* api_end_subject */         NULL,

    /* api_start_attributes */    NULL,
    /* api_end_attributes */      NULL,

    /* api_start_container */     NULL,
    /* api_end_container */       api1_end_container,

    /* api_start_state */         NULL,
    /* api_end_state */           NULL,

    /* api_act */                 NULL,
    /* api_subject */             NULL,
    /* api_attributes */          NULL,

    /* api_sep */                 api_sep,
    /* api_token */               api1_token,
    /* api_string */              api_string,

    /* api_frag */                NULL,

    /* api_error */               print_error
};

emit_t *emit_g_api = &api;
emit_t *emit_g_api1 = &api1;
