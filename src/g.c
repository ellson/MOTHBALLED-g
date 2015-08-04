#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

# include "list.h"
# include "emit.h"
# include "parse.h"
# include "dumpg.h"

int main (int argc, char *argv[]) {
    int i, opt, optnum, needstdin, rc;
    FILE *f;
    static context_t context;

    emit = emit_g_api;  // default emitter

    while ((opt = getopt(argc, argv, "d::g::t::")) != -1) {
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
        default:
            fprintf(stderr,
		"Usage: %s [-d[01] | [-t[01]] | [-g[01]] [files] [-]  \n",
		argv[0]
	    );
            exit(1);
        }
    }

    needstdin = 1;        // with no args, read stdin
    for (i=optind; i<argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            needstdin = 1;    // with explicit '-' as last file arg, read stdin
	    break;
        }
        else {
            needstdin = 0;   // indicate have args, so no default to reading stdin unless '-'
            f = fopen(argv[i],"r");
	    if (f) {
                rc = parse(&context, f);
                fclose(f);
                if (rc) exit(rc);
	    }
	    else {
		fprintf(stderr, "file \"%s\" is not readable\n", argv[i]);
		exit(1);
	    }
        }
    }
    if (needstdin) {
        rc = parse(&context, stdin);
        if (rc) exit(rc);
    }

    return 0;
}
