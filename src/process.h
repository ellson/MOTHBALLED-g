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

struct process_s {
    THREAD_t *THREAD;          // THREADs in this PROCESS

// FIXME - use enumeration
    int flags;       // -s = 1, -p = 2, -c = 4, -g = 8
    char *acts;      // g snippet from command line

    // the following require mutex to allow sharing from threads
    PROC_INBUF_t PROC_INBUF; // inbuf free_list and stats
    PROC_LIST_t PROC_LIST;   // elem free_list and stats
    elem_t *identifiers; // tree of identifiers
    ikea_store_t *ikea_store; // persistency 
    elem_t *merge_cache;  // cache of hashes from previous merges

    // info collected by process();
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
