#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"
#include "info.h"
#include "parse.h"
#include "persist.h"
#include "dumpg.h"

static emit_t *emitters[] = {&g_api, &g1_api, &g2_api, &t_api, &t1_api, &gv_api};

static context_t context;  // the input context - needs to be global for intr()

// if interrupted we try to gracefully snapshot the current state 
static void intr(int s)
{
    (void) s; // NOTUSED

    je_persist_snapshot(&context);
    je_persist_close(&context);
    exit (EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int i, opt, optnum, needstats = 0, needrestore = 0;
    emit_t *ep;
    elem_t *name;

    context.progname = argv[0];

    emit = &g_api;      // default output engine (-Tg)

    signal(SIGINT, intr);

	while ((opt = getopt(argc, argv, "T:d::g::t::sr")) != -1) {
		if (optarg)
			optnum = atoi(optarg);
		else
			optnum = 0;
		switch (opt) {
        case 'T':
            for (i = 0; i < SIZEOF_EMITTERS; i++) {
                ep = emitters[i];
                if (strcmp(optarg, ep->name) == 0) {
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
			fprintf(stderr, "Usage: %s [-d[01] | [-s] [-t[01]] | [-g[01]] [files] [-]  \n", context.progname);
			exit(EXIT_FAILURE);
		}
	}

	argv = &argv[optind];
	argc -= optind;

	if (argc == 0) {	// No file args,  default to stdin
		argv[0] = "-";
		argc++;
	}

    context.pargc = &argc;
    context.argv = argv;

    // gather session info, including starttime for stats
    je_session(&context);   // FIXME - move to je_persist_open() ??
    
    // assemble a name and create temp folder for this nameless top container
    name = je_persist_open(&context);

    if (needrestore) {
        je_persist_restore(&context);
    }

    emit_start_parse(&context);
    je_parse(&context, name);
    emit_end_parse(&context);

    // generate snapshot
    je_persist_snapshot(&context);

    // and stats, if wanted     // FIXME - why not keep these in the tempdir so save automatically
    if (needstats) {
        // FIXME - need pretty-printer
        fprintf (stderr, "%s\n", je_session(&context));
        fprintf (stderr, "%s\n", je_stats(&context));
    }

    je_persist_close(&context);

	exit(EXIT_SUCCESS);
}
