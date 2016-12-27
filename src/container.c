/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "thread.h"
#include "container.h"
#include "info.h"
#include "process.h"

/**
 * @param THREAD context   
 * @return success/fail
 */
success_t container(THREAD_t * THREAD)
{
    CONTAINER_t container = { 0 };
    elem_t *root;
    success_t rc;

    root = new_list(LIST(), ACTIVITY);

    container.THREAD = THREAD;

    TOKEN()->previous = new_list(LIST(), SUBJECT);  // for sameas

    THREAD->stat_containdepth++;      // containment nesting level
    if (THREAD->stat_containdepth > THREAD->stat_containdepthmax) {
        THREAD->stat_containdepthmax = THREAD->stat_containdepth;
    }
    container.stat_containercount++;    // number of containers in this container

    container.ikea_box = ikea_box_open(THREAD->ikea_store, NULL);

    if ((rc = parse(&container, root, 0, SREP, 0, 0, NLL)) == FAIL) {
        if (TOKEN()->insi == NLL) {    // EOF is OK
            rc = SUCCESS;
        } else {
            token_error(TOKEN(), "Parse error near token:", TOKEN()->insi);
        }
    }

//P(THREAD->identifiers);

    if (container.nodes) {
//        ikea_box_append(ikea_box, data, data_len)
        printt(THREAD, container.nodes);
P(container.nodes);

    }
    if (container.edges) {
//        ikea_box_append(ikea_box, data, data_len)
        printt(THREAD, container.edges);
    }

// FIXME - don't forget to include NODE and EDGE patterns, after NODES and EDGES
//   (Patterns are in effect now, but may not have been at the creation of existing objects.)

    ikea_box_close ( container.ikea_box );

    THREAD->stat_containdepth--;

    free_list(LIST(), root);
    free_list(LIST(), TOKEN()->previous);
    free_tree(LIST(), container.nodes);
    free_tree(LIST(), container.edges);
    free_tree(LIST(), container.node_patterns);
    free_tree(LIST(), container.edge_patterns);
// Some elem's are retained by the attrid tree
//E();

    // FIXME - move to Aunt Sally query
    if (THREAD->PROCESS->needstats) {
        info_process(THREAD);
        info_thread(THREAD);
        info_container(&container);
    }

    return rc;
}
