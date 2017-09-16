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

#define DUMMY_LOCK()
#define DUMMY_UNLOCK()

struct process_s {
    THREAD_t *THREAD;          // THREADs in this PROCESS

// FIXME - use enumeration
    int flags;       // -s = 1, -p = 2, -c = 4, -g = 8
    char *acts;      // g snippet from command line

    elem_t *identifiers;       // tree of identifiers  - requires mutex when accessing from threads

    ikea_store_t *ikea_store;  // persistency - may require mutex ??  depends, I think, on if rename() is atomic

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
