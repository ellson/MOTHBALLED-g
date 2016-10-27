/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#include "emit.h"

static void api_start_activity(CONTENT_t * CONTENT)
{
    PARSE_t * PARSE = CONTENT->PARSE;

    PARSE->sep = 0;
    if (PARSE->containment != 0) {
        putc('{', PARSE->out);
    }
}

static void api_end_activity(CONTENT_t * CONTENT)
{
    PARSE_t * PARSE = CONTENT->PARSE;

    PARSE->sep = 0;
    if (PARSE->containment != 0) {
        putc('}', PARSE->out);
    }
    else {
        putc('\n', PARSE->out);
    }
}

static void api_subject(CONTENT_t * CONTENT, elem_t *list)
{
    PARSE_t * PARSE = CONTENT->PARSE;

    je_emit_list(PARSE, PARSE->out, list);
}

static void api_attributes(CONTENT_t * CONTENT, elem_t *list)
{
    PARSE_t * PARSE = CONTENT->PARSE;

    je_emit_list(PARSE, PARSE->out, list);
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
    PARSE_t * PARSE = CONTENT->PARSE;

    if (PARSE->containment == 0) {
        putc('\n', PARSE->out);
    }
}

static void api1_sep(PARSE_t * PARSE)
{
    putc(' ', PARSE->out);
}

static void api1_string(PARSE_t * PARSE, elem_t * branch)
{
    char sep;

    sep = 0;
    print_list(PARSE->out, branch, -1, &sep);
}

static void api1_token(PARSE_t * PARSE, char token)
{
    putc(token, PARSE->out);
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
    putc('\n', CONTENT->PARSE->out);
}

static void api2_token(PARSE_t * PARSE, char token)
{
    putc('\n', PARSE->out);
    putc(token, PARSE->out);
    putc(' ', PARSE->out);
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
