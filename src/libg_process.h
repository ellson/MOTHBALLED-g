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

#ifndef LIBG_PROCESS_H
#define LIBG_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

// process.c
void process(int *pargc, char *argv[], int optind, char needstats);

// emit.c
int select_emitter(char *name);

// dumpg.c
void set_sstyle( void );
void printgrammar0( void );
void printgrammar1( void );

#ifdef __cplusplus
}
#endif

#endif
