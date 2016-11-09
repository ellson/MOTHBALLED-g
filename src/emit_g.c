/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#include "emit.h"

static void api_start_activity(CONTAINER_t * CONTAINER)
{
    CONTAINER_t * CONTAINER = (CONTAINER_t*)CONTAINER;

    CONTAINER->sep = 0;
    if (CONTAINER->containment != 0) {
        putc('{', CONTAINER->out);
    }
}

static void api_end_activity(CONTAINER_t * CONTAINER)
{
    CONTAINER_t * CONTAINER = (CONTAINER_t*)CONTAINER;

    CONTAINER->sep = 0;
    if (CONTAINER->containment != 0) {
        putc('}', CONTAINER->out);
    }
    else {
        putc('\n', CONTAINER->out);
    }
}

static void api_subject(CONTAINER_t * CONTAINER, elem_t *list)
{
    CONTAINER_t * CONTAINER = (CONTAINER_t*)CONTAINER;

    je_emit_list(CONTAINER, CONTAINER->out, list);
}

static void api_attributes(CONTAINER_t * CONTAINER, elem_t *list)
{
    CONTAINER_t * CONTAINER = (CONTAINER_t*)CONTAINER;

    je_emit_list(CONTAINER, CONTAINER->out, list);
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

static void api1_end_activity(CONTAINER_t * CONTAINER)
{
    CONTAINER_t * CONTAINER = (CONTAINER_t*)CONTAINER;

    if (CONTAINER->containment == 0) {
        putc('\n', CONTAINER->out);
    }
}

static void api1_sep(CONTAINER_t * CONTAINER)
{
    putc(' ', CONTAINER->out);
}

static void api1_string(CONTAINER_t * CONTAINER, elem_t * branch)
{
    char sep;

    sep = 0;
    print_elem(CONTAINER->out, branch, -1, &sep);
}

static void api1_token(CONTAINER_t * CONTAINER, char token)
{
    putc(token, CONTAINER->out);
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

static void api2_end_activity(CONTAINER_t * CONTAINER)
{
    CONTAINER_t * CONTAINER = (CONTAINER_t*)CONTAINER;

    putc('\n', CONTAINER->out);
}

static void api2_token(CONTAINER_t * CONTAINER, char token)
{
    putc('\n', CONTAINER->out);
    putc(token, CONTAINER->out);
    putc(' ', CONTAINER->out);
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
