/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "libg_session.h"

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
    int opt, optnum, needstats = 0;

    signal(SIGINT, intr);

    while ((opt = getopt(argc, argv, "d::s")) != -1) {
        if (optarg)
            optnum = atoi(optarg);
        else
            optnum = 0;
        switch (opt) {
        case 'd':
            switch (optnum) {
            case 0:
                dumpg();
                exit(EXIT_SUCCESS);
                break;
            case 1:
                printg();
                exit(EXIT_SUCCESS);
                break;
            default:
                fprintf(stderr,"%s\n", "-d0 = linear walk, -d1 = recursive walk");
                exit(EXIT_FAILURE);
                break;
            }

            break;
        case 's':
            needstats = 1;
            break;
        default:
            fprintf(stderr,"Usage: %s [-d[01] [-s] [files] [-]\n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    // create the top-level context for processing the inputs
    session(&argc, argv, optind, needstats);

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
 * args ::= '-?;
 *          '-V'
 *          '-P' [fmt= out=]
 *          '-Q' [fmt= out=]
 *          '-C' [fmt= out= lay=]
 *          '-i' [fmt=] {in in in ...}
 *          '+i' [fmt=] {in in in ...}
 *          '-F' {args}
 *
 * defaults:
 *         args:  -Q[fmt=g out=-]
 *         fmt:   g
 *         lay:   fdp
 *         out:   -    
 *         in:    -
 */
