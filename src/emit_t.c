/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#include "emit.h"

static void api_start_activity(CONTENT_t * CONTENT)
{
    CONTEXT_t *C = CONTENT->C;
    FILE *chan = C->out;

    C->sep = 0;
    if (C->containment == 0) {
        fprintf(chan, "// |-- on entry, '|' alt, '.' one, '?' zero or one, '*' zero or more, '+' one or more\n");
        fprintf(chan, "// |-- on exit, '0' success, '1' fail\n");
        fprintf(chan, "//     |-- state number\n");
        fprintf(chan, "//         |-- nesting\n");
        fprintf(chan, "//             |-- iteration\n");
        fprintf(chan, "//                 |-- state name\n");
        fprintf(chan, "//                              |-- string (if present)\n");
    }
}

static void
api_start_state(CONTENT_t * CONTENT, char class, unsigned char prop, int nest, int repc)
{
    FILE *chan = CONTENT->C->out;

    fprintf(chan, "\n   ");
    putc(je_char_prop(prop, '.'), chan);
    fprintf(chan, "%4d%4d%4d   ", class, nest, repc);
    print_len_frag(chan, NAMEP(class));
}

static void api_string(CONTEXT_t * C, elem_t * branch)
{
    FILE *chan = C->out;
    char sep;

    sep = 0;
    putc('\t', chan);
    print_list(chan, branch, -1, &sep);
}

static void api_token(CONTEXT_t * C, char token)
{
    FILE *chan = C->out;

    fprintf(chan, "\t\t%c", token);
}

static void
api_end_state(CONTENT_t * CONTENT, char class, success_t rc, int nest, int repc)
{
    FILE *chan = CONTENT->C->out;

    fprintf(chan, "\n   %d%4d%4d%4d   ", rc, class, nest, repc);
    print_len_frag(chan, NAMEP(class));
}

static void api_end_activity(CONTENT_t * CONTENT)
{
    CONTEXT_t *C = CONTENT->C;
    FILE *chan = C->out;

    C->sep = 0;
    putc('\n', chan);
}

emit_t t_api = { "t",
    /* api_initialize */ NULL,
    /* api_finalize */ NULL,

    /* api_start_file */ NULL,
    /* api_end_file */ NULL,

    /* api_start_activity */ api_start_activity,
    /* api_end_activity */ api_end_activity,

    /* api_start_act */ NULL,
    /* api_end_act */ NULL,

    /* api_start_subject */ NULL,
    /* api_end_subject */ NULL,

    /* api_start_state */ api_start_state,
    /* api_end_state */ api_end_state,

    /* api_act */ NULL,
    /* api_subject */ NULL,
    /* api_attributes */ NULL,

    /* api_sep */ NULL,
    /* api_token */ api_token,
    /* api_string */ api_string,

    /* api_frag */ NULL,
};

static void api1_act(CONTENT_t * CONTENT, elem_t * tree)
{
    FILE *chan = CONTENT->C->out;
    char sep;

    sep = ' ';
    fprintf(chan, "%3d ACT", CONTENT->C->containment);
    print_list(chan, tree, 7, &sep);
}

emit_t t1_api = { "t1",
    /* api_start_parse */ NULL,
    /* api_end_parse */ NULL,

    /* api_start_file */ NULL,
    /* api_end_file */ NULL,

    /* api_start_activity */ NULL,
    /* api_end_activity */ api_end_activity,

    /* api_start_act */ NULL,
    /* api_end_act */ NULL,

    /* api_start_subject */ NULL,
    /* api_end_subject */ NULL,

    /* api_start_state */ NULL,
    /* api_end_state */ NULL,

    /* api_act */ api1_act,
    /* api_subject */ NULL,
    /* api_attributes */ NULL,

    /* api_sep */ NULL,
    /* api_token */ NULL,
    /* api_string */ NULL,

    /* api_frag */ NULL,
};
