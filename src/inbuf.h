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

#ifndef INBUF_H
#define INBUF_H

#ifdef __cplusplus
extern "C" {
#endif

// My guess is that INBUFSIZE should match the average ACT size, so that
// inbufs get released roughly as ACTs are processed.
// I could be wrong - it may need experimentation.
//
// patterns will hold inbufs for a long lime,  so inbufs
// shouldn't be much bigger than the average pattern.
//
// small inbufs cause greater fragmentation of strings
// small inbufs have greater overhead ratio.

// sizeof(inbufelem_t) = 1<<7  (128 bytes)
// the contents size is sizeof(inbufelem_t) less the other bits  (~116 bytes)
#define INBUFSIZE ((1<<7) - sizeof(inbufelem_t*) - sizeof(int))
#define INBUFALLOCNUM 128

struct inbufelem_s {
    inbufelem_t *nextinbuf;
    int refs;
    unsigned char buf[INBUFSIZE];
};

struct inbuf_s {
    inbufelem_t *inbuf;            // the active input buffer
    inbufelem_t *free_inbuf_list;  // linked list of unused inbufs
    long stat_inbufmalloc;
    long stat_inbufmax;
    long stat_inbufnow;
};

inbufelem_t * new_inbuf(INBUF_t * INBUF);
void free_inbuf(INBUF_t * INBUF, inbufelem_t * inbuf);

#ifdef __cplusplus
}
#endif

#endif
