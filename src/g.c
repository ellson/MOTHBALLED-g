#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "emit.h"
#include "info.h"
#include "parse.h"
#include "dumpg.h"

static emit_t *emitters[] = {&g_api, &g1_api, &g2_api, &t_api, &t1_api, &gv_api};

int main(int argc, char *argv[])
{
	int i, opt, optnum, needstats;
    emit_t *ep;
    context_t context = { 0 };  // the input context

    // set some defaults
    context.out = stdout;
    context.err = stderr;

    emit = &g_api;      // default output engine (-T)
	needstats = 0;		// stats default to no stats

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
                fprintf(context.err, "No back-end found for format: -T%s\n", optarg);
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
				fprintf(context.err, "%s\n", "-d0 = linear walk, -d1 = recursive walk");
				exit(EXIT_FAILURE);
				break;
			}

			break;
		case 's':
			needstats = 1;
			break;
		default:
			fprintf(context.err, "Usage: %s [-d[01] | [-s] [-t[01]] | [-g[01]] [files] [-]  \n", argv[0]);
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
    parse(&context, needstats);
    emit_end_parse(&context);

	// any errors in parse() will be handled by emit_error().  If we get here
	// then exit with success
	exit(EXIT_SUCCESS);

}
