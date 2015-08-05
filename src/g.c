#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

# include "inbuf.h"
# include "emit.h"
# include "parse.h"
# include "dumpg.h"

static context_t context;

int main (int argc, char *argv[]) {
    int i, opt, optnum, needstdin, needstats, rc;
    FILE *f;
    context_t *C;

    C = &context;
    emit = emit_g_api;  // default emitter
    needstats = 0;          // no default stats

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

    rc = 0;
    needstdin = 1;        // with no args, read stdin
    for (i=optind; i<argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            needstdin = 1;    // with explicit '-' as last file arg, read stdin
	    break;
        }
        else {
            needstdin = 0;   // indicate have args, so no default to reading stdin unless '-'
            C->filename = argv[i];
            f = fopen(argv[i],"r");
	    if (f) {
		C->file = f;
                rc = parse(&context);
                fclose(f);
                if (rc) break;
	    }
	    else {
		fprintf(stderr, "file \"%s\" is not readable\n", argv[i]);
		exit(1);
	    }
        }
    }
    if (!rc) {
        if (needstdin) {
            C->filename = "-";
            C->file = stdin;
            rc = parse(&context);
        }
    }

    if (needstats) {
        fprintf(stdout, "\nStats:  (tbd)\n");
    }

    exit(rc);
}
