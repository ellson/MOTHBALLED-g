/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "thread.h"
#include "container.h"
#include "info.h"
#include "session.h"

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
    container.previous_subject = new_list(LIST(), SUBJECT);  // for sameas
    container.patterns = new_list(LIST(), 0);  // for patterns

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

    if (container.nodes) {
        THREAD->sep = ' ';
        print_elem(THREAD, container.nodes, 0);

//        ikea_box_append(ikea_box, data, data_len)

    }
    if (container.edges) {
//        ikea_box_append(ikea_box, data, data_len)

        THREAD->sep = ' ';
        print_elem(THREAD, container.edges, 0);
    }

// FIXME - don't forget to include NODE and EDGE patterns, after NODES and EDGES
//   (Paterns are in effect now, but may not have been at the creation of existing objects.)

    ikea_box_close ( container.ikea_box );

    THREAD->stat_containdepth--;

    if (THREAD->SESSION->needstats) {
        fprintf(TOKEN()->out, "\n%s\n", info_session(&container));
        fprintf(TOKEN()->out, "%s\n", info_stats(&container));
    }

    free_list(LIST(), root);
    free_tree(LIST(), container.nodes);
    free_tree(LIST(), container.edges);
    free_list(LIST(), container.previous_subject);
    free_list(LIST(), container.patterns);

    if (LIST()->stat_elemnow) {
        E();
        assert(LIST()->stat_elemnow == 0);   // check for elem leaks
    }

    return rc;
}
