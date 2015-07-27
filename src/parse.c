#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "list.h"
#include "parse.h"

#define OUT stdout
#define ERR stderr

#define styleLAN sstyle?"< ":"<"
#define styleRAN sstyle?"\n>\n":">"
#define styleLPN sstyle?"\n  ( ":"("
#define styleRPN sstyle?"\n  )\n":")"
#define styleLBR sstyle?"\n  [ ":"["
#define styleRBR sstyle?"\n  ]\n":"]"
#define styleLBE sstyle?"\n  { ":"{"
#define styleRBE sstyle?"\n  }\n":"}"

static int sstyle=0;

void set_sstyle (void) { 
    sstyle=1;
}

#define PROPP(p) (state_props + (p - state_machine))

static char *get_name(char *p) {
    int offset;
    while (*p) p++;
    offset = *PROPP(p) * 2;
    return state_names + offset;
}

static char *oleg1=NULL, *oleg2=NULL;
static char *sleg1, *sleg2;

static void print_next( char *leg1, char *leg2 ) {
    if (leg1 != oleg1) {
	oleg1 = leg1;
        sleg1 = get_name(leg1);
        oleg2 = NULL;
    }
    else {
	sleg1 = "=";
    }
    if (leg2 != oleg1) {
	oleg2 = leg2;
        sleg2 = get_name(leg2);
    }
    else {
	sleg2 = "=";
    }
    fprintf(OUT,"%s%s %s%s", styleLAN, sleg1, sleg2, styleRAN);
}

static void print_attr ( char attr, char *attrid, int *inlist ) {
    if (attr) {
	if ((*inlist)++) putc (' ', OUT);
        fprintf(OUT,attrid);
    }
}

static void print_prop(char *p) {
    unsigned char prop;
    int inlist;

    prop = *PROPP(p);
    if (prop & (ALT|OPT|SREP|REP)) {
        inlist=0;
        fprintf(OUT,"%s", styleLBR);
        print_attr( prop & ALT, "ALT", &inlist);
        print_attr( prop & OPT, "OPT", &inlist);
        print_attr( prop & SREP, "SREP", &inlist);
        print_attr( prop & REP, "REP", &inlist);
        fprintf(OUT,"%s", styleRBR);
    }
}

static void printg_r(char *sp, int indent) {
    char *p, *np, nxt;
    int i;

    p = sp;
    while (( nxt = *p )) {
        np = p+nxt;
        for (i = indent; i--; ) putc (' ', OUT);
        print_next(sp, np);
        print_prop(p);
        fprintf(OUT,"%s\n", styleLBE);
	if (np != state_machine) { // stop recursion
            printg_r(np, indent+2);
        }
        for (i = indent; i--; ) putc (' ', OUT);
        fprintf(OUT,"%s\n", styleRBE);

        p++;
    }
}

// recursively walk the grammar - tests all possible transitions
void printg (void) {
    printg_r(state_machine, 0);
}

static void print_chars ( char *p ) {
    int i, cnt, si;

    si = (p - state_machine);
    cnt=0;
    for (i=0; i<0x100; i++) {
        if (si == char2state[i]) {
	    if (cnt++) {
	        putc (' ', OUT);
	    }
	    else {
		fprintf(OUT,"%s",styleLBE);
	    }
            fprintf(OUT,"%02x", i);
	}
    }
    if (cnt++) {
	fprintf(OUT,"%s",styleRBE);
    }
}

// just dump the grammar linearly,  should result in same logical graph as printg()
void dumpg (void) {
    char *p, *sp, nxt;

    p = state_machine;
    while (p < (state_machine + sizeof_state_machine)) {
        if (*p) { // non-terminal
            sp = p;
            while (( nxt = *p )) {
                print_next(sp, p+nxt);
                print_prop(p);
		p++;
	    }
	}
	else { // else terminal
	    fprintf(OUT,"%s", get_name(p));
            print_chars(p);
	}
        p++;
        fprintf(OUT,"\n");
    }
}

static unsigned char c, *in;
static char *insp;

static int parse_r(char *sp, int indent) {
    unsigned char prop;
    char *p, *np, nxt;
    int i, rc;

#if 1
    fprintf(OUT, "\n");
    for (i = indent; i--; ) putc (' ', OUT);
    fprintf(OUT,"%s ", get_name(sp));
#endif

    if (insp == NULL) {
        c = *in++;
        if (!c) {
            fprintf(OUT,"EOF\n");
	    return 1;
        }
        insp = state_machine + char2state[c];
    }
    if (insp == sp) {
#if 1
        fprintf(OUT, "%c", c);
#endif
        insp = NULL;
	return 0;
    }

    rc = 1;
    p = sp;
    while (( nxt = *p )) {
        prop = *PROPP(p);
        np = p + nxt;

	if ( (prop & ALT)) {
	    if (( rc = parse_r(np, indent+2) ) != 0) {
                p++;
		continue;
	    }
	} 
	if (!(prop & OPT)) {
	    if (( rc = parse_r(np, indent+2) ) != 0) {
		break; 
	    }
	}
	if ( (prop & (REP|SREP))) {
	    while (( rc = parse_r(np, indent+2) ) == 0) { }
	    p++;
	    break;
	}
	if ( (prop & OPT)) {
	    if (( rc = parse_r(np, indent+2) ) == 0) {
		p++;
		break; 
	    }
	}
        p++;
    }
    return rc;
}

int parse(unsigned char *input) {
    int rc;

    in = input;
    rc = parse_r(state_machine, 0);
    if (rc) {
        fprintf(OUT,"ERROR\n");
    }
    return rc;
}


#if 0
typedef struct {
    elem_t
	fraglist,       // string fragments forming a single string
	npathlist,      // strings components forming a path
	nlistlist,      // paths descibing sets of nodes or endpoints
        edgelist,       // list of endpoints sets describing one or more edges
        elistlist,      // list of edges or of edge sets
    	nproplist,      // list of node properties
    	eproplist,      // list of edge properties
	cproplist,      // list of container properties
        containerlist;  // list of acts
} act_t;

static int opencloseblock(act_t *act, state_t state) {
    return 0;
}

int parse(unsigned char *insp) {
    unsigned char c;
    int rc, linecharcnt, linecnt, charsize;
    int state;
    act_t *act;
    elem_t *fraglist, *npathlist, *nlistlist /*, *edgelist, *elistlist,
	*nproplist, *eproplist, *cproplist, *containerlist */;
    elem_t *frag, *npathelem, *nlistelem;

    act = calloc(sizeof(act_t), 1);

    fraglist = &(act->fraglist);
    npathlist = &(act->npathlist);
    nlistlist = &(act->nlistlist);
/*  edgelist = &(act->edgelist);
    elistlist = &(act->elistlist);
    nproplist = &(act->nproplist);
    eproplist = &(act->eproplist);
    cproplist = &(act->cproplist);
    containerlist = &(act->containerlist); */

    frag = NULL;
    linecharcnt = 0;
    linecnt = 0;
    while ((c = *insp++)) {
        linecharcnt++;
        state = char2state[c];
        charsize = 1;
        if (state & TWO) {
            if ((c = *insp)) {
	        state = char2state[c];

                // we work out what the character pair is by combining the 
                // charpros from both chars  (this means that CRLF is only a single EOL)
                // and then masking out the TWO and other unrelated bits
                state |= charTWOstate2props[state];
                state &= proptypemask;
                if (state) {
		    insp++;
                    charsize++;
	            if ((state & EOL)) {
                        linecnt++;
                        linecharcnt = 0;
                    }
		}
	    }
        }
        if (frag) {    // a frag already exists from a previous iteration of the while loop
            assert (frag->type == STR);
            if (state & frag->state) {  // matching state, just continue appending to frag
                frag->u.str.len += charsize;
            }
	    else {
                if (state & fraglist->state) {  // if matches fraglist 
 		    if (state & (STRING|SPACE)) {  // STRING or SPACE char that can be simply appended
                        frag->u.str.len += charsize;
                    }
                    else if (state & ESCAPE) {  // STRING frag that starts at this char - needs a new frag to skip BSL char
			frag = newelem(STRING, insp-1, 1, 0);
			appendlist(fraglist, frag);
                    }
                    else { // DQTSTR|SQTSTR -- prop that start at next char - so new frag to skip this char
			frag = newelem(state & proptypemask, insp, 0, 0);
			appendlist(fraglist, frag);
                    }
                }
 		else { // new state don't match, so something got terminated and needs promoting
//printj(fraglist);
                    if (fraglist->state & (STRING|ESCAPE)) {
			npathelem = list2elem(fraglist);
			appendlist(npathlist, npathelem);
//printj(npathlist);

//FIXME - something in here to deal with path separators:  :/ <abc>/ <abc> :: <abc> : <abc>
			nlistelem = list2elem(npathlist);
			appendlist(nlistlist, nlistelem);
printj(nlistlist);
		    } 
                    freelist(fraglist);  // free anything that hasn't been promoted,  e.g. WS
                    frag = NULL;
		}
            }
        }

        if (state & (OPEN|CLOSE)) {
	    rc = opencloseblock(act, state);
	    if (rc) {
		fprintf(ERR,"parser error at: %d:%d \"%c\"\n", linecnt,  linecharcnt, c);
                return rc;
            }
        }
// FIXME - deal with OPEN and CLOSE
//     NPATH | NLIST --> EDGE
//     EDGE --> ELIST

        if (!frag) { // no current frag
            if (state & (STRING|ESCAPE)) {
		fraglist->state = (STRING|ESCAPE);
		frag = newelem(STRING, insp-1, 1, 0);
		appendlist(fraglist, frag);
            }
            else if (state & (DQTSTR|SQTSTR)) {
		fraglist->state = (STRING|ESCAPE);
		frag = newelem(STRING, insp, 0, 0);
		appendlist(fraglist, frag);
            }
	    else if (state & SPACE) {
		fraglist->state = SPACE;
		frag = newelem(SPACE, insp-1, 1, 0);
		appendlist(fraglist, frag);
	    }
// FIXME - something for quoted strings
	}
    }
    return 0;
}
#endif
