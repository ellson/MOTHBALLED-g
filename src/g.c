#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

# include "inbuf.h"
# include "emit.h"
# include "parse.h"
# include "dumpg.h"

static context_t context;

int main (int argc, char *argv[]) {
    int i, rc, opt, optnum, needstdin, needstats;
    FILE *f;
    context_t *C;
    struct timespec starttime;

    C = &context;
    emit = emit_g_api;       // default emitter
    needstats = 0;           // stats default to no stats

    rc = clock_gettime(CLOCK_MONOTONIC_RAW, &starttime);
    assert(rc == 0);
    filecount = 0;
    actcount = 0;

    while ((opt = getopt(argc, argv, "d::g::t::s")) != -1) {
	if (optarg) optnum = atoi(optarg);
	else optnum = 0;
        switch (opt) {
        case 'd':
            switch (optnum) {
	    case 0: dumpg(); exit(0); break;
	    case 1: printg(); exit(0); break;
	    default:
		fprintf(stderr, "%s\n",
		    "-d0 = linear walk, -d1 = recursive walk"
		);
		exit(1);
		break;
	    }
            
            break;
        case 'g':
            switch (optnum) {
	    case 0: emit = emit_g_api; break;
	    case 1: emit = emit_g_api1; break;
	    default:
		fprintf(stderr, "%s\n",
		    "-g0 = minimal space, -g1 = shell-friendly spacing"
		);
		exit(1);
		break;
	    }
            break;
        case 't':
            switch (optnum) {
	    case 0: emit = emit_t_api; break;
	    case 1: emit = emit_t_api1; break;
	    default:
		fprintf(stderr, "%s\n",
		    "-t0 = input parse, -t1 = output tree"
		);
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
		argv[0]
	    );
            exit(1);
        }
    }

    needstdin = 1;           // with no args, read stdin
    for (i=optind; i<argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            needstdin = 1;   // with explicit '-' as last file arg, read stdin
	    break;
        }
        else {
            needstdin = 0;   // indicate have args, so do not default
			     // to reading stdin unless '-' is explicitly given
            C->filename = argv[i];
            f = fopen(argv[i],"r");
	    if (f) {
                filecount++;
		C->file = f;
                parse(&context);
                fclose(f);
	    }
	    else {
		fprintf(stderr, "file \"%s\" is not readable\n", argv[i]);
		exit(1);
	    }
        }
    }
    if (needstdin) {
        filecount++;
        C->filename = "-";
        C->file = stdin;
        parse(&context);
    }

    if (needstats) {
        print_stats(stderr, &starttime);
    }

    // any errors in parse() will be handled by emit_error().  If we get here
    // then exit with success
    exit(0);
}
