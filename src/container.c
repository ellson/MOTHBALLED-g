/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "container.h"

/**
 * parse G syntax input
 *
 * This parser recurses at two levels:
 *
 * parse() --> container() --> graph() -----| -|  
 *           ^               ^   |          |  |
 *           |               |   -> doact() |  |
 *           |               |              |  |
 *           |               --------<------|  |
 *           |                                 |
 *           ----------------<-----------------|
 *
 * The outer recursions are through nested containment.
 *
 * The inner recursions are through the grammar state_machine at a single
 * level of containment - maintained in the CONTAINER context
 *
 * The top-level SESSION context is available to both and maintains the input state.
 *
 * @param GRAPH context
 * @return success/fail
 */
success_t container(GRAPH_t * GRAPH)
{
    CONTAINER_t container_context = { 0 };
    CONTAINER_t * CONTAINER = &container_context;
    success_t rc;
    TOKEN_t * TOKEN = (TOKEN_t *)GRAPH;
    LIST_t * LIST = (LIST_t *)GRAPH;
    elem_t *root = new_list(LIST, ACTIVITY);

    CONTAINER->GRAPH = GRAPH;
    CONTAINER->subject = new_list(LIST, SUBJECT);
    CONTAINER->node_pattern_acts = new_list(LIST, 0);
    CONTAINER->edge_pattern_acts = new_list(LIST, 0);
    CONTAINER->ikea_box = ikea_box_open(GRAPH->ikea_store, NULL);
    CONTAINER->out = stdout;
    emit_start_activity(CONTAINER);
    GRAPH->containment++;            // containment nesting level
    GRAPH->stat_containercount++;    // number of containers

    if ((rc = graph(CONTAINER, root, ACTIVITY, SREP, 0, 0)) == FAIL) {
        if (TOKEN->insi == NLL) {    // EOF is OK
            rc = SUCCESS;
        } else {
            token_error(TOKEN, TOKEN->state, "Parse error. Last good state was:");
        }
    }
    if (CONTAINER->nodes) {
        GRAPH->sep = ' ';
        print_tree(GRAPH->out, CONTAINER->nodes, &(GRAPH->sep));
        putc('\n', GRAPH->out);
    }
    if (CONTAINER->edges) {
        GRAPH->sep = ' ';
        print_tree(GRAPH->out, CONTAINER->edges, &(GRAPH->sep));
        putc('\n', GRAPH->out);
    }

// FIXME - don't forget to include NODE and EDGE patterns, after NODES and EDGES
//   (Paterns are in effect now, but may not have been at the creation of existing objects.)

    GRAPH->containment--;
    emit_end_activity(CONTAINER);

    ikea_box_close ( CONTAINER->ikea_box );

    free_list(LIST, root);
    free_tree(LIST, CONTAINER->nodes);
    free_tree(LIST, CONTAINER->edges);
    free_list(LIST, CONTAINER->subject);
    free_list(LIST, CONTAINER->node_pattern_acts);
    free_list(LIST, CONTAINER->edge_pattern_acts);

    if (LIST->stat_elemnow) {
        E();
        assert(LIST->stat_elemnow == 0);   // check for elem leaks
    }

    return rc;
}
