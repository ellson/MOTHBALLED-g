/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "thread.h"
#include "container.h"
#include "iter_print.h"
#include "info.h"
#include "process.h"

static size_t stdout_writer(const void *ptr, size_t size)
{
    return fwrite(ptr, size, 1, stdout);
}

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

    container.previous = new_list(LIST(), SUBJECT);  // for sameas

    THREAD->stat_containdepth++;      // containment nesting level
    if (THREAD->stat_containdepth > THREAD->stat_containdepthmax) {
        THREAD->stat_containdepthmax = THREAD->stat_containdepth;
    }
    container.stat_containercount++;    // number of containers in this container

    if ((rc = parse(&container, root, 0, SREP, 0, 0, END)) == FAIL) {
        if (TOKEN()->insi == END) {    // EOF is OK
            rc = SUCCESS;
        } else {
            token_error(TOKEN(), "Parse error near token:", TOKEN()->insi);
        }
    }


    if (container.nodes) {
        THREAD->ikea_box = ikea_box_open( THREAD->ikea_store, NULL);
        THREAD->writer_fn = &stdout_writer;
        ikea_printt (THREAD, container.nodes);
        THREAD->writer_fn = &stdout_writer;
        printt(THREAD, container.nodes);
        if (container.edges) {
            THREAD->writer_fn = &stdout_writer;
            ikea_printt (THREAD, container.edges);
            THREAD->writer_fn = &stdout_writer;
            printt(THREAD, container.edges);
        }
        THREAD->writer_fn = &stdout_writer;
        ikea_box_close ( THREAD->ikea_box,
                THREAD->contenthash, sizeof(THREAD->contenthash) );
    }

//    if (container.node_patterns) {
//        ikea_box_append(ikea_box, data, data_len)
//        printt(THREAD, container.edges);
//    }
//    if (container.edge_patterns) {
//        ikea_box_append(ikea_box, data, data_len)
//        printt(THREAD, container.edges);
//    }


    THREAD->stat_containdepth--;

    free_list(LIST(), root);
    free_list(LIST(), container.previous);
    free_tree(LIST(), container.nodes);
    free_tree(LIST(), container.edges);
    free_tree(LIST(), container.node_patterns);
    free_tree(LIST(), container.edge_patterns);
// Some elem's are retained by the attrid tree
//E();

    // FIXME - move to Aunt Sally query
    if (THREAD->PROCESS->needstats) {
        // in alpha-sorted order
        info_container(&container);
        info_process(THREAD);
        info_thread(THREAD);
    }

    return rc;
}
