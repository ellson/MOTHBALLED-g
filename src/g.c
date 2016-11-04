/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>

#include "success.h"
#include "libg_session.h"
#include "fatal.h" 

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
    int opt, optnum, needstats = 0, needrestore = 0;

    signal(SIGINT, intr);

    while ((opt = getopt(argc, argv, "T:d::g::t::sr")) != -1) {
        if (optarg)
            optnum = atoi(optarg);
        else
            optnum = 0;
        switch (opt) {
#if 0
        case 'T':
            if (select_emitter(optarg) == FAIL) {
                fprintf(stderr, "No back-end found for format: -T%s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
#endif
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
        case 'r':
            needrestore = 1;
            break;
        default:

// FIXME no -g or -t support any more ??
// FIXME need something like -p  (print primary content)
// FIXME need something like -P "name" (print content of "name")
// FIXME "name" should allow full subject string, or "hashname"
// FIXME need something like -i (print tree index of container "hashname" and "SUBJECT")
// FIXME - add -T options to usage message
// FIXME - do -i processing here so can index restored pre-parsed graphs
// FIXME - allow multiple -p in loop
// FIXME - do -p / -P processing here so can print resored pre-parsed graphs
//
            fprintf(stderr,"Usage: %s [-d[01] | [-s] [-t[01]] | [-g[01]] [files] [-]  \n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    // create the top-level context for processing the inputs
    session(&argc, argv, optind);

    exit(EXIT_SUCCESS);
}
