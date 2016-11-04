/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>

#include "libje.h"
#include "fatal.h" 

static THREAD_t * THREAD;  // the context - needs to be global for intr()

// if interrupted we try to gracefully snapshot the current state 
static void intr(int s)
{
    (void) s; // NOTUSED

    interrupt(THREAD);

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
        case 'T':
            if (select_emitter(optarg) == FAIL) {
                fprintf(stderr, "No back-end found for format: -T%s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
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
    THREAD = initialize(&argc, argv, optind);

    // parse the input 
    parse(THREAD);

    // and stats, if wanted 
    if (needstats) {
        // FIXME - need pretty-printer
        fprintf (stderr, "%s\n", session(THREAD));
        fprintf (stderr, "%s\n", stats(THREAD));
    }

    finalize(THREAD);

    exit(EXIT_SUCCESS);
}
