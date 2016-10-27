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
    fprintf(PARSE->out, "graph {\n");
}

static void api_end_activity(CONTENT_t * CONTENT)
{
    PARSE_t * PARSE = CONTENT->PARSE;

    PARSE->sep = 0;
    fprintf(PARSE->out, "\n}\n");
}

static void api_list(CONTENT_t * CONTENT, elem_t *list)
{
    PARSE_t * PARSE = CONTENT->PARSE;

    je_emit_list(PARSE, PARSE->out, list);
}

emit_t gv_api = { "gv",
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
    /* api_subject */ api_list,
    /* api_attributes */ api_list,

    /* api_sep */ NULL,
    /* api_token */ NULL,
    /* api_string */ NULL,

    /* api_frag */ NULL,
};
