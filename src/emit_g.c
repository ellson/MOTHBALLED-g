/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#include "emit.h"

static void api_start_activity(CONTENT_t * CONTENT)
{
    CONTEXT_t *C = CONTENT->C;

    C->sep = 0;
    if (C->containment != 0) {
        putc('{', C->out);
    }
}

static void api_end_activity(CONTENT_t * CONTENT)
{
    CONTEXT_t *C = CONTENT->C;

    C->sep = 0;
    if (C->containment != 0) {
        putc('}', C->out);
    }
    else {
        putc('\n', C->out);
    }
}

static void api_subject(CONTENT_t * CONTENT, elem_t *list)
{
    CONTEXT_t *C = CONTENT->C;

    je_emit_list(C, C->out, list);
}

static void api_attributes(CONTENT_t * CONTENT, elem_t *list)
{
    CONTEXT_t *C = CONTENT->C;

    je_emit_list(C, C->out, list);
}

emit_t g_api = { "g",
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

    /* api_start_state */ NULL,
    /* api_end_state */ NULL,

    /* api_act */ NULL,
    /* api_subject */ api_subject,
    /* api_attributes */ api_attributes,

    /* api_sep */ NULL,
    /* api_token */ NULL,
    /* api_string */ NULL,

    /* api_frag */ NULL,
};

static void api1_end_activity(CONTENT_t * CONTENT)
{
    CONTEXT_t *C = CONTENT->C;

    if (C->containment == 0) {
        putc('\n', C->out);
    }
}

static void api1_sep(CONTEXT_t * C)
{
    putc(' ', C->out);
}

static void api1_string(CONTEXT_t *C, elem_t * branch)
{
    char sep;

    sep = 0;
    print_list(C->out, branch, -1, &sep);
}

static void api1_token(CONTEXT_t *C, char token)
{
    putc(token, C->out);
}

emit_t g1_api = { "g1",
    /* api_initialize */ NULL,
    /* api_finalize */ NULL,

    /* api_start_file */ NULL,
    /* api_end_file */ NULL,

    /* api_start_activity */ NULL,
    /* api_end_activity */ api1_end_activity,

    /* api_start_act */ NULL,
    /* api_end_act */ NULL,

    /* api_start_subject */ NULL,
    /* api_end_subject */ NULL,

    /* api_start_state */ NULL,
    /* api_end_state */ NULL,

    /* api_act */ NULL,
    /* api_subject */ NULL,
    /* api_attributes */ NULL,

    /* api_sep */ api1_sep,
    /* api_token */ api1_token,
    /* api_string */ api1_string,

    /* api_frag */ NULL,
};

static void api2_end_activity(CONTENT_t * CONTENT)
{
    putc('\n', CONTENT->C->out);
}

static void api2_token(CONTEXT_t * C, char token)
{
    putc('\n', C->out);
    putc(token, C->out);
    putc(' ', C->out);
}

emit_t g2_api = { "g2",
    /* api_initialize */ NULL,
    /* api_finalize */ NULL,

    /* api_start_file */ NULL,
    /* api_end_file */ NULL,

    /* api_start_activity */ NULL,
    /* api_end_activity */ api2_end_activity,

    /* api_start_act */ NULL,
    /* api_end_act */ NULL,

    /* api_start_subject */ NULL,
    /* api_end_subject */ NULL,

    /* api_start_state */ NULL,
    /* api_end_state */ NULL,

    /* api_act */ NULL,
    /* api_subject */ NULL,
    /* api_attributes */ NULL,

    /* api_sep */ api1_sep,
    /* api_token */ api2_token,
    /* api_string */ api1_string,

    /* api_frag */ NULL,
};
