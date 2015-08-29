#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "libje.h"

static emit_t *emit;  // the output plugin
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
	int i, opt, optnum, needstats = 0, needrestore = 0;
    emit_t *ep;
    elem_t *name;

    emit = emitters[0];      // default output engine (-Tg)

    signal(SIGINT, intr);

	while ((opt = getopt(argc, argv, "T:d::g::t::sr")) != -1) {
		if (optarg)
			optnum = atoi(optarg);
		else
			optnum = 0;
		switch (opt) {
        case 'T':
            for (i = 0; i < SIZEOF_EMITTERS; i++) {
                if ((ep = je_emit_name_match(optarg, emitters[i]))) 
                {
                    emit = ep;
                    break;
                }
            }
            if (i >= SIZEOF_EMITTERS) {
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


// libje usage starts here...

    C = je_initialize();
    C->progname = argv[0];
    C->out = stdout;

	argv = &argv[optind];
	argc -= optind;

	if (argc == 0) {	// No file args,  default to stdin
		argv[0] = "-";
		argc++;
	}

    C->pargc = &argc;
    C->argv = argv;

    // gather session info, including starttime for stats
    je_session(C);   // FIXME - move to je_persist_open() ??
    
    // assemble a name and create temp folder for this nameless top container
    name = je_persist_open(C);

    if (needrestore) {
        je_persist_restore(C);
    }

    // start parsing
    je_parse(C, name);

    // generate snapshot
    je_persist_snapshot(C);

    // and stats, if wanted     // FIXME - why not keep these in the tempdir so save automatically
    if (needstats) {
        // FIXME - need pretty-printer
        fprintf (stderr, "%s\n", je_session(C));
        fprintf (stderr, "%s\n", je_stats(C));
    }

    je_persist_close(C);
    je_finalize(C);

	exit(EXIT_SUCCESS);
}
