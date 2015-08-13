#include <stdio.h>
#include <stdlib.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"

static void api_start_activity(context_t *C) {
    if (C->containment == 0) {
        fprintf(OUT, "// |-- on entry, '|' alt, '.' one, '?' zero or one, '*' zero or more, '+' one or more\n");
        fprintf(OUT, "// |-- on exit, '0' success, '1' fail\n");
        fprintf(OUT, "//     |-- state number\n");
        fprintf(OUT, "//         |-- nesting\n");
        fprintf(OUT, "//             |-- iteration\n");
        fprintf(OUT, "//                 |-- state name\n");
        fprintf(OUT, "//                              |-- string (if present)\n");
    }
}

static void api_start_state(context_t *C, char class, unsigned char prop, int nest, int repc) {
    fprintf(OUT, "\n   ");
    putc(char_prop(prop,'.'), OUT);
    fprintf(OUT,"%4d%4d%4d   ", class, nest, repc);
    print_len_frag(OUT, NAMEP(class));
}

static void api_string(context_t *C, elem_t *branch) {
    fprintf(OUT,"\t\"");
    print_list(OUT, branch, -1, 0);
    putc('"', OUT);
}

static void api_token(context_t *C, char token) {
    fprintf(OUT,"\t\t'%c'", token);
}

static void api_end_state(context_t *C, char class, success_t rc, int nest, int repc) {
    fprintf(OUT,"\n   %d%4d%4d%4d   ", rc, class, nest, repc);
    print_len_frag(OUT, NAMEP(class));
}

static void api_end_activity(context_t *C) {
    if (C->containment == 0) {
        fprintf(OUT, "\n");
    }
}

static emit_t api = {
    /* api_start_file */          NULL,
    /* api_start_activity */      api_start_activity,
    /* api_sep */                 NULL,
    /* api_start_state */         api_start_state,
    /* api_act */                 NULL,
    /* api_subject */             NULL,
    /* api_string */              api_string,
    /* api_frag */                NULL,
    /* api_token */               api_token,
    /* api_end_state */           api_end_state,
    /* api_term */                NULL,
    /* api_end_activity */        api_end_activity,
    /* api_end_file */            NULL,
    /* api_error */               print_error
};

static void api1_subject(context_t *C, elem_t *tree) {
    fprintf(OUT,"%3d ",C->containment);
    print_list(OUT, tree, 4, ' ');
}

static void api1_term(context_t *C) {
//    fprintf(OUT,"\n(term)\n");
    putc('\n', OUT);
}

static emit_t api1 = {
    /* api_start_file */          NULL,
    /* api_start_activity */      NULL,
    /* api_sep */                 NULL,
    /* api_start_state */         NULL,
    /* api_act */                 NULL,
    /* api_subject */             api1_subject,
    /* api_string */              NULL,
    /* api_frag */                NULL,
    /* api_token */               NULL,
    /* api_end_state */           NULL,
    /* api_term */                api1_term,
    /* api_end_activity */        api_end_activity,
    /* api_end_file */            NULL,
    /* api_error */               print_error
};

emit_t *emit_t_api = &api;
emit_t *emit_t_api1 = &api1;
