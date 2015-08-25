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
#include "dumpg.h"

#define NOTUSED(var)    (void) var

static emit_t *emitters[] = {&g_api, &g1_api, &g2_api, &t_api, &t1_api, &gv_api};

static context_t context;  // the input context

static void intr(int s)
{
    /* if interrupted we try to gracefully suspend the current state */

    NOTUSED(s);

    context.suspend = 1;
    g_suspend(&context);
    
    exit (EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	int i, opt, optnum;
    emit_t *ep;

    context.progname = argv[0];

    emit = &g_api;      // default output engine (-Tg)

    signal(SIGINT, intr);

	while ((opt = getopt(argc, argv, "T:d::g::t::s")) != -1) {
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
			context.needstats = 1;
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

    emit_start_parse(&context);
    g_parse(&context, NULL);
    emit_end_parse(&context);

	exit(EXIT_SUCCESS);
}
