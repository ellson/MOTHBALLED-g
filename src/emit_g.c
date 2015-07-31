#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "list.h"
#include "emit.h"

#if 0
static void api_start_state_machine(context_t *C) {
    putc(char_prop(0,'_'), OUT);
}
#endif

static void api_sep(context_t *C) {
    putc (' ', OUT);
}

static void api_string(context_t *C, elem_t *string) {
//    print_list(OUT, string);
}

static void api_frag(context_t *C, unsigned char len, unsigned char *frag) {
    print_frag(OUT, len, frag);
}

static void api1_tok(context_t *C, char class, unsigned char len, unsigned char *frag) {
    print_frag(OUT, len, frag);
}

static void api2_tok(context_t *C, char class, unsigned char len, unsigned char *frag) {
    putc('\n', OUT);
    print_frag(OUT, len, frag);
    putc(' ', OUT);
}

#if 0
static void api_end_state(context_t *C, char class, int rc, int nest) {
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

static emit_t api1 = {
    /* api_start_state_machine */ NULL,
    /* api_sep */                 api_sep,
    /* api_start_state */         NULL,
    /* api_string */              api_string,
    /* api_frag */                api_frag,
    /* api_tok */                 api1_tok,
    /* api_end_state */           NULL,
    /* api_term */                NULL,
    /* api_end_state_machine */   api_end_state_machine,
    /* api_error */               api_error
};

static emit_t api2 = {
    /* api_start_state_machine */ NULL,
    /* api_sep */                 api_sep,
    /* api_start_state */         NULL,
    /* api_string */              api_string,
    /* api_frag */                api_frag,
    /* api_tok */                 api2_tok,
    /* api_end_state */           NULL,
    /* api_term */                NULL,
    /* api_end_state_machine */   api_end_state_machine,
    /* api_error */               api_error
};

emit_t *emit_g_api1 = &api1;
emit_t *emit_g_api2 = &api2;
