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

#ifndef ITER_H
#define ITER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXNEST 20

typedef struct {
    elem_t *lnx;
    char **sep;
} lnx_t;

typedef struct {
    elem_t *tnx;
    int dir;
} tnx_t;

typedef struct {
    lnx_t lstack[MAXNEST];
    tnx_t tstack[MAXNEST];
    uint16_t lsp;
    uint16_t tsp;
    uint16_t len;
    int pretty;  // 0 = minimal spacing, 2 = pretty spacing 
    unsigned char *cp, intree;
} iter_t;

int compare(elem_t * a, elem_t * b);
int match(elem_t * a, elem_t * b);
void printt(IO_t * IO, elem_t * p);
void printg(IO_t * IO, elem_t * p);

#ifdef __cplusplus
}
#endif

#endif
