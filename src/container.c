/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "thread.h"
#include "print.h"
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
 * The top-level THREAD context is available to both and maintains the input state.
 *
 * @param THREAD context   
 * @return success/fail
 */
success_t container(THREAD_t * THREAD)
{
    CONTAINER_t container = { 0 };
    TOKEN_t * TOKEN = (TOKEN_t *)THREAD;
    LIST_t * LIST = (LIST_t *)TOKEN;
    elem_t *root;
    success_t rc;

    root = new_list(LIST, ACTIVITY);

    container.THREAD = THREAD;
    container.previous_subject = new_list(LIST, SUBJECT);  // for sameas

    container.node_pattern_acts = new_list(LIST, 0);  // FIXME use tree
    container.edge_pattern_acts = new_list(LIST, 0);  // FIXME use tree

    container.ikea_box = ikea_box_open(THREAD->ikea_store, NULL);

    THREAD->stat_containdepth++;      // containment nesting level
    if (THREAD->stat_containdepth > THREAD->stat_containdepthmax) {
        THREAD->stat_containdepthmax = THREAD->stat_containdepth;
    }
    THREAD->stat_containcount++;    // number of containers

    if ((rc = graph(&container, root, ACTIVITY, SREP, 0, 0)) == FAIL) {
        if (TOKEN->insi == NLL) {    // EOF is OK
            rc = SUCCESS;
        } else {
            token_error(TOKEN, "Parse error. Last good state was:", TOKEN->bi);
        }
    }

    if (container.nodes) {
        THREAD->sep = ' ';
        print_elem(THREAD, container.nodes, 0);
    }
    if (container.edges) {
        THREAD->sep = ' ';
        print_elem(THREAD, container.edges, 0);
    }

// FIXME - don't forget to include NODE and EDGE patterns, after NODES and EDGES
//   (Paterns are in effect now, but may not have been at the creation of existing objects.)

    THREAD->stat_containdepth--;
    ikea_box_close ( container.ikea_box );

    free_list(LIST, root);
    free_tree(LIST, container.nodes);
    free_tree(LIST, container.edges);
    free_list(LIST, container.previous_subject);
    free_list(LIST, container.node_pattern_acts);
    free_list(LIST, container.edge_pattern_acts);

    if (LIST->stat_elemnow) {
        E();
        assert(LIST->stat_elemnow == 0);   // check for elem leaks
    }

    return rc;
}
