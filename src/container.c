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
#include <string.h>
#include <assert.h>

#include "thread.h"
#include "container.h"
#include "info.h"
#include "process.h"

//===================================

static void* stdout_open(void *descriptor, char *mode) {
    return stdout;
}

static size_t stdout_write(THREAD_t *THREAD, unsigned char *cp, size_t len) {
    return fwrite(cp, len, 1, stdout);
}

static void stdout_flush(THREAD_t *THREAD) {
    return;
}

static void stdout_close(THREAD_t *THREAD) {
    return;
}

// discipline for writing to stdout
static out_disc_t stdout_disc = {
    &stdout_open,
    &stdout_write,
    &stdout_flush,
    &stdout_close
};

//===================================

#if 0  // not used yet
static void* file_open(void *descriptor, char *mode) {
    return fopen((char*)(descriptor), mode);
}

static size_t file_write(THREAD_t *THREAD, unsigned char *cp, size_t len) {
    return fwrite(cp, len, 1, (FILE*)(THREAD->out));
}

static void file_flush(THREAD_t *THREAD) {
    fflush((FILE*)(THREAD->out));
}

static void file_close(THREAD_t *THREAD) {
    fclose((FILE*)(THREAD->out));
}

// discipline for writing to a file
static out_disc_t file_disc = {
    &file_open,
    &file_write,
    &file_flush,
    &file_close
};
#endif

//===================================

static void* ikea_open(void* descriptor, char *mode) {
    return ikea_box_open( (ikea_store_t*)descriptor, mode);
}

static void ikea_flush(THREAD_t *THREAD) {
    ikea_box_append((ikea_box_t*)(THREAD->out), THREAD->buf, THREAD->pos);
    THREAD->pos = 0;
}

static size_t ikea_write(THREAD_t *THREAD, unsigned char *cp, size_t size)
{
    int len = size;

    while (len--) {
        THREAD->buf[THREAD->pos++] = *cp++;
        if (THREAD->pos >= sizeof(THREAD->buf)) {
            ikea_flush(THREAD);
        }
    }
    return size;
}

static void ikea_close(THREAD_t *THREAD) {
    ikea_box_close ( (ikea_box_t*)(THREAD->out),
            THREAD->contenthash,
            sizeof(THREAD->contenthash) );
}


// discipline for writing to ikea
static out_disc_t ikea_disc = {
    &ikea_open,
    &ikea_write,
    &ikea_flush,
    &ikea_close 
};

//===================================

/**
 * @param THREAD context   
 * @return success/fail
 */
success_t container(THREAD_t * THREAD)
{
    CONTAINER_t container = { 0 };
    elem_t *root;
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

    THREAD->pretty = 1;

    if ((rc = parse(&container, root, 0, SREP, 0, 0, END)) == FAIL) {
        if (TOKEN()->insi == END) {    // EOF is OK
            rc = SUCCESS;
        } else {
            token_error(TOKEN(), "Parse error near token:", TOKEN()->insi);
        }
    }
    


    // FIXME - write to stdout - should be based on a query
    if (container.nodes) {
        THREAD->out_disc = &stdout_disc;
        THREAD->out = THREAD->out_disc->out_open_fn( NULL, NULL );
        printt(THREAD, container.nodes);
        if (container.edges) {
            printt(THREAD, container.edges);
        }
        THREAD->out_disc->out_flush_fn(THREAD);
        THREAD->out_disc->out_close_fn(THREAD);
    }

    // preserve in ikea storage
    if (container.nodes) {
        THREAD->out_disc = &ikea_disc;
        THREAD->out = THREAD->out_disc->out_open_fn( THREAD->ikea_store, NULL);
        printt(THREAD, container.nodes);
        if (container.edges) {
            printt(THREAD, container.edges);
        }
        THREAD->out_disc->out_flush_fn(THREAD);
        THREAD->out_disc->out_close_fn(THREAD);
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
