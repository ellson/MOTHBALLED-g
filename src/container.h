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

#ifndef CONTAINER_H
#define CONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

struct container_s {      // CONTAINER context
    THREAD_t *THREAD;     // parent THREAD

    state_t verb;         // verb for the current ACT
    state_t subj_has_ast; // flag set if '*' occurred in SUBJECT
    state_t attr_has_ast; // flag set if '*' occurred in ATTRIBUTES
    state_t has_sameas;   // flag set if '=' occurred in SUBJECT
    state_t has_mum;      // flag set if '^' occurred in SUBJECT
    state_t has_node;     // flag set if NODE occurred in SUBJECT
    state_t has_edge;     // flag set if EDGE occurred in SUBJECT

    elem_t *previous;     // previous ACT for sameas
    elem_t *nodes;        // tree of unique NODEs
    elem_t *edges;        // tree of unique EDGEs

    // stats
    long stat_containercount;
    long stat_inactcount;
    long stat_sameas;
    long stat_outactcount;
};

success_t container(THREAD_t *THREAD);
elem_t *playpen(THREAD_t *THREAD, elem_t *node);

#ifdef __cplusplus
}
#endif

#endif
