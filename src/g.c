/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>

#include "libje.h"
#include "fatal.h"   // FIXME - so is this public or private?
                     // or should option processing be in a private function?

static context_t *C;  // the input context - needs to be global for intr()

// if interrupted we try to gracefully snapshot the current state 
static void intr(int s)
{
    (void) s; // NOTUSED

    ikea_store_snapshot(C);
    ikea_store_close(C);
    exit (EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int opt, optnum, needstats = 0, needrestore = 0;
    elem_t *name;

    signal(SIGINT, intr);

    while ((opt = getopt(argc, argv, "T:d::g::t::sr")) != -1) {
        if (optarg)
            optnum = atoi(optarg);
        else
            optnum = 0;
        switch (opt) {
        case 'T':
            if (je_select_emitter(optarg) == FAIL)
                fatal_printf("No back-end found for format: -T%s\n", optarg);
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
                fatal_printf("%s\n", "-d0 = linear walk, -d1 = recursive walk");
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
            fatal_printf("Usage: %s [-d[01] | [-s] [-t[01]] | [-g[01]] [files] [-]  \n", argv[0]);
            break;
        }
    }

    // create the top-level context for processing the inputs
    C = je_initialize(&argc, argv, optind);

    // assemble a name for the top container in the form of
    // a fraglist, and create temp folder for the per-container files
    name = ikea_store_open(C);

    if (needrestore) {
        ikea_store_restore(C);
    }

    // start parsing
    je_parse(C, name);

//  FIXME - do -i processing here so can index restored pre-parsed graphs
//  FIXME - allow multiple -p in loop
//  FIXME - do -p / -P processing here so can print resored pre-parsed graphs

    // generate snapshot
    ikea_store_snapshot(C);

    // and stats, if wanted 
    if (needstats) {
        // FIXME - need pretty-printer
        fprintf (stderr, "%s\n", je_session(C));
        fprintf (stderr, "%s\n", je_stats(C));
    }

    ikea_store_close(C);
    je_finalize(C);
    exit(EXIT_SUCCESS);
}
