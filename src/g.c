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
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "libg_process.h"

// if interrupted we try to gracefully snapshot the current state 
static void intr(int s)
{
    (void) s; // NOTUSED

#if 0
// FIXME - once we know what we  really want from command line interrupts
    interrupt(THREAD);
#endif
    exit (EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int opt, optnum, flags = 0;
    char *acts = NULL;

    signal(SIGINT, intr);

    while ((opt = getopt(argc, argv, "d::pcsg:")) != -1) {
        if (optarg)
            optnum = atoi(optarg);
        else
            optnum = 0;
        switch (opt) {
        case 'd':   // debug
            switch (optnum) {
            case 0:
                printgrammar0();
                exit(EXIT_SUCCESS);
                break;
            case 1:
                printgrammar1();
                exit(EXIT_SUCCESS);
                break;
            default:
                fprintf(stderr,"%s\n", "-d0 = linear walk, -d1 = recursive walk");
                exit(EXIT_FAILURE);
                break;
            }

            break;
        case 's':    // stats
            flags |= 1;
            break;
        case 'p':    // pretty output
            flags |= 2;
            break;
        case 'c':    // expand contents
            flags |= 4;
            break;
        case 'g':    // eval g acts from command line
                     // evaluated after all named files, 
                     // but before explicit '-'
                     // no implicit stdin processing if -g present
            flags |= 8;
            acts = optarg;
            break;
        default:
            fprintf(stderr,"Usage: %s [-d[01] [-s] [-p] [-c] [files] [-g acts] [-]\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    // create the top-level context for processing the inputs
    process(&argc, argv, optind, flags, acts);

    exit(EXIT_SUCCESS);
}


/**
 *
 * FIXME - g should accept options in g syntax
 *
 * Usage:   g <args>
 *
 * Where <args> is a list of option "nodes" in g syntax:
 *
 * args ::= '-?'                       -- help
 *          '-V'                       -- version
 *          '-S' [is= es= ly= os= ot=] -- io stream
 *          '-G' {args}                -- fork off a separate graph
 
 * defaults:
 *         args:  -S[is=- es=- ly=- os=- ot=-]
 *
 *         ly:    none      (or: '-', dot, fdp, neato, ...)
 *         is:    stdin     (or: '-', file, :socket, namedpipe| ) 
 *         os:    stdout    (or: '-', file, :socket, namedpipe| )
 *         es:    stderr    (or: '-', file, :socket, namedpipe| )
 *         ot:    g         (or: gv, svg, png, ...)
 *
 *         file       ::= file | /unix/path/file | C:\dos\path\file
 *         socket     ::= :num
 *         namedpipe  ::= name|
 *
 * example:
 *         filter:   -S[is=- es=- os=- ot=g]
 */
