#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "libje.h"

static context_t *C;  // the input context - needs to be global for intr()

// if interrupted we try to gracefully snapshot the current state 
static void intr(int s)
{
    (void) s; // NOTUSED

    je_persist_snapshot(C);
    je_persist_close(C);
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
            if (je_select_emitter(optarg) == FAIL) {
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
				fprintf(stderr, "%s\n", "-d0 = linear walk, -d1 = recursive walk");
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
// FIXME - add -T options to usage message
			fprintf(stderr, "Usage: %s [-d[01] | [-s] [-t[01]] | [-g[01]] [files] [-]  \n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}


    // create the top-level context for processing the inputs
    C = je_initialize(argc, argv, optind);

    // assemble a name for the top container in the form of a fraglist, and create temp folder for the per-container files
    name = je_persist_open(C);

    if (needrestore) {
        je_persist_restore(C);
    }

    // start parsing
    je_parse(C, name);

    // generate snapshot
    je_persist_snapshot(C);

    // and stats, if wanted 
    if (needstats) {
        // FIXME - need pretty-printer
        fprintf (stderr, "%s\n", je_session(C));
        fprintf (stderr, "%s\n", je_stats(C));
    }

    je_persist_close(C);

    je_finalize(C);

	exit(EXIT_SUCCESS);
}
