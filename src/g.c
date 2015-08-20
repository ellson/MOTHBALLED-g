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
#include "stats.h"
#include "emit.h"
#include "parse.h"
#include "dumpg.h"

int main(int argc, char *argv[])
{
	success_t rc;
	int opt, optnum, needstats;
    int i;
	struct timespec starttime;
	FILE *out, *err;
    emit_t *ep;

	rc = clock_gettime(CLOCK_MONOTONIC_RAW, &starttime);
	assert(rc == SUCCESS);

	out = stdout;
	err = stderr;
    emit = &g_api;      // default emitter
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
                fprintf(stderr, "No back-end found for format: -T%s\n", optarg);
                exit(1);
            }
            break;
		case 'd':
			switch (optnum) {
			case 0:
				dumpg();
				exit(0);
				break;
			case 1:
				printg();
				exit(0);
				break;
			default:
				fprintf(stderr, "%s\n",
					"-d0 = linear walk, -d1 = recursive walk");
				exit(1);
				break;
			}

			break;
		case 's':
			needstats = 1;
			break;
		default:
			fprintf(stderr,
				"Usage: %s [-d[01] | [-s] [-t[01]] | [-g[01]] [files] [-]  \n",
				argv[0]);
			exit(1);
		}
	}

	argv = &argv[optind];
	argc -= optind;

	if (argc == 0) {	// No file args,  default to stdin
		argv[0] = "-";
		argc++;
	}

	rc = parse(&argc, argv, out, err);

	if (needstats) {
		print_stats(err, &starttime);
	}
	// any errors in parse() will be handled by emit_error().  If we get here
	// then exit with success
	exit(SUCCESS);

}
