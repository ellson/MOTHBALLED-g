/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#include "emit.h"

static void api_start_activity(CONTAINER_t * CONTAINER)
{
    GRAPH_t * GRAPH = (GRAPH_t*)CONTAINER;

    GRAPH->sep = 0;
    fprintf(GRAPH->out, "graph {\n");
}

static void api_end_activity(CONTAINER_t * CONTAINER)
{
    GRAPH_t * GRAPH = (GRAPH_t*)CONTAINER;

    GRAPH->sep = 0;
    fprintf(GRAPH->out, "\n}\n");
}

static void api_list(CONTAINER_t * CONTAINER, elem_t *list)
{
    GRAPH_t * GRAPH = (GRAPH_t*)CONTAINER;

    je_emit_list(GRAPH, GRAPH->out, list);
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
