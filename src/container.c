/* vim:set shiftwidth=4 ts=8 expandtab: */

/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "thread.h"
#include "container.h"
#include "info.h"
#include "process.h"

/**
 * @param THREAD context   
 * @return SUCCESS or FAIL
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

    if ((rc = parse(&container, root, 0, REP, 0, 0, END)) == FAIL) {
        if (TOKEN()->insi == END) {    // EOF is OK
            rc = SUCCESS;
        } else {
            token_error(TOKEN(), "Parse error near token:", TOKEN()->insi);
        }
    }

    // preserve in ikea storage
    if (container.nodes) {
        IO()->out_disc = &ikea_disc;
        IO()->out_chan = IO()->out_disc->out_open_fn( THREAD->PROCESS->ikea_store, NULL);
        printt(IO(), container.nodes);
        if (container.edges) {
            printt(IO(), container.edges);
        }
        IO()->out_disc->out_flush_fn(IO());
        IO()->out_disc->out_close_fn(IO());
    }

    THREAD->stat_containdepth--;

    free_list(LIST(), root);
    free_list(LIST(), container.nodes);
    free_list(LIST(), container.edges);
    free_list(LIST(), container.previous);

// Some elem's are retained by the attrid tree
//E();

    // FIXME - move to Aunt Sally query
    if (THREAD->PROCESS->flags & 4) {
        // in alpha-sorted order
        info_container(&container);
        info_process(THREAD);
        info_thread(THREAD);
    }

    return rc;
}

/**
 * @param THREAD context   
 * @param node with kid
 * @return node with kid in playpen
 */
elem_t * playpen(THREAD_t * THREAD, elem_t *node)
{
//    P(node);
    return node;
}
