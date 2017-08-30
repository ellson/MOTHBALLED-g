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

#ifndef PROCESS_H
#define PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#define TEN9 1000000000
#define TEN3 1000

// Graph Object Model
typedef struct {
    elem_t *nodes;    // tree of nodes (sorted, unique)
    elem_t *edges;    // tree of edges (sorted, unique)
} GOM_t;

struct process_s {
    THREAD_t *THREAD;          // THREADs in this PROCESS

// FIXME - replace with a properly formed QRY
    char needstats;            // flag set if -s on command line
//
    GOM_t MUM;       // Primary, in-memory graph  (one of the sisters, never explicitly named)
    GOM_t SALLY;     // Aunt Sally,    Statistics  
    GOM_t GUDRUN;    // Aunt Gudrun,   Grammar  (devine knowledge)

    // info collected by session();
    char *progname;
    char *username;
    char *hostname;          
    char *osname;
    char *osrelease;
    char *osmachine;
    uint64_t pid;
    uint64_t uptime;
    uint64_t uptime_nsec;
    uint64_t starttime;
    uint64_t starttime_nsec;
};

#ifdef __cplusplus
}
#endif

#endif
