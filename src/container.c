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

static void subject_tree2acts(LIST_t *LIST, elem_t *activity, elem_t * p)
{
    if (p->u.t.left) {
        subject_tree2acts(LIST, activity, p->u.t.left);
    }
    elem_t *act = new_list(LIST, ACT);
    append_addref(act, p->u.t.key);
    append_transfer(activity, act);
    if (p->u.t.right) {
        subject_tree2acts(LIST, activity, p->u.t.right);
    }
}

/**
 * @param THREAD context   
 * @return content
 */
elem_t * container(THREAD_t * THREAD)
{
    CONTAINER_t container = { 0 };
    elem_t *root, *content = NULL;
    success_t rc;


// FIXME need to maintain the pathname of the container.
//      pathname and content hash need to be stored in ikea
//      when opening a container, initialize with content from ikea

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

    content = new_list(LIST(), ACTIVITY);
    if (container.nodes) {
        subject_tree2acts(LIST(), content, container.nodes);
        if (container.edges) {
            subject_tree2acts(LIST(), content, container.edges);
        }
    }
    
#ifndef NEWPRINT
    // write to stdout
    if (THREAD->stat_containdepth == 1) {
        if (container.nodes) {
            IO()->flags = THREAD->PROCESS->flags;
            IO()->out_disc = &stdout_disc;
            IO()->out_chan = IO()->out_disc->out_open_fn( NULL, NULL );
            printt(IO(), container.nodes);
            if (container.edges) {
                printt(IO(), container.edges);
            }
            IO()->out_disc->out_flush_fn(IO());
            IO()->out_disc->out_close_fn(IO());
        }
    }
    else {
        if (THREAD->PROCESS->flags & 8) {
            // FIXME
            fprintf(stderr, "printing of expanded content is not yet implemented\n");
        }
    }
#endif

    // preserve in ikea storage
    if (container.nodes) {
        IO()->out_disc = &ikea_disc;
        IO()->out_chan = IO()->out_disc->out_open_fn( IO()->ikea_store, NULL);
        printt(IO(), container.nodes);
        if (container.edges) {
            printt(IO(), container.edges);
        }
        IO()->out_disc->out_flush_fn(IO());
        IO()->out_disc->out_close_fn(IO());
    }

    THREAD->stat_containdepth--;

    free_list(LIST(), root);
    free_list(LIST(), container.previous);
    free_tree(LIST(), container.nodes);
    free_tree(LIST(), container.edges);

// Some elem's are retained by the attrid tree
//E();

    // FIXME - move to Aunt Sally query
    if (THREAD->PROCESS->flags & 4) {
        // in alpha-sorted order
        info_container(&container);
        info_process(THREAD);
        info_thread(THREAD);
    }

    return content;
}
