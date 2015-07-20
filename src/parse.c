#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.c"
#include "parse.h"
#include "list.h"

static void printg_next(int s, int indent) {
    int *pn, ss, n, nn, i;
    char *s_name, *n_name;

    ss = s & 0xFF;
    s_name = *state_name[ss];
    pn = state_next[ss];
    while (1) {
        n = *pn++;
	nn = n & 0xFF;        
        if (!nn) break;
        n_name = *state_name[nn];
        for (i = indent; i--; ) putc (' ', stdout);
	printf("< %s %s > {\n", s_name, n_name);
	if (n & REC) continue;
        if ((n ^ s) & TWO) continue;
        printg_next(n, indent+2);
        for (i = indent; i--; ) putc (' ', stdout);
	printf("}\n");
    }
}

void printg (void) {
    printg_next(ACT, 0);
}

static unsigned char *inp, c;
static int in;

static int parse_next(int s) {
    int *pn, n, nn, rc;

#if 1
    int ss;
    char *s_name;
#endif
#if 2
    char *in_name;
#endif

#if 1
    ss = s & 0xFF;
    s_name = *state_name[ss];
    printf("%s\n", s_name);
#endif

    pn = state_next[ss];
    while (1) {
        n = *pn++;
        nn = n & 0xFF;
        if (!nn) {
	    if ( (in & 0xFF) == ss ) {
#if 2
                in_name = *state_name[in];
		printf("    %s\n", in_name);
#endif
	     c = *inp++;
             in = char2state[c];
	     return 0;
	  }
          return 1;
	}
    
//        if (n & REC) continue;     // FIXME - maybe allocate new inp buffers?

        if ((n ^ s) & TWO) continue;

	if (n & REP) { while (( rc = parse_next(n) ) == 0); break; }
	if (n & ALT) if (( rc = parse_next(n) ) != 0) continue;
	if (n & OPT) if (( rc = parse_next(n) ) == 0) break;
        rc = parse_next(n);
    }
    return rc;
}

int parse(unsigned char *input) {
    inp = input;
    c = *inp++;
    in = char2state[c];
    return parse_next(ACT);
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

int parse(unsigned char *inp) {
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
    while ((c = *inp++)) {
        linecharcnt++;
        state = char2state[c];
        charsize = 1;
        if (state & TWO) {
            if ((c = *inp)) {
	        state = char2state[c];

                // we work out what the character pair is by combining the 
                // charpros from both chars  (this means that CRLF is only a single EOL)
                // and then masking out the TWO and other unrelated bits
                state |= charTWOstate2props[state];
                state &= proptypemask;
                if (state) {
		    inp++;
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
			frag = newelem(STRING, inp-1, 1, 0);
			appendlist(fraglist, frag);
                    }
                    else { // DQTSTR|SQTSTR -- prop that start at next char - so new frag to skip this char
			frag = newelem(state & proptypemask, inp, 0, 0);
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
		fprintf(stderr, "parser error at: %d:%d \"%c\"\n", linecnt,  linecharcnt, c);
                return rc;
            }
        }
// FIXME - deal with OPEN and CLOSE
//     NPATH | NLIST --> EDGE
//     EDGE --> ELIST

        if (!frag) { // no current frag
            if (state & (STRING|ESCAPE)) {
		fraglist->state = (STRING|ESCAPE);
		frag = newelem(STRING, inp-1, 1, 0);
		appendlist(fraglist, frag);
            }
            else if (state & (DQTSTR|SQTSTR)) {
		fraglist->state = (STRING|ESCAPE);
		frag = newelem(STRING, inp, 0, 0);
		appendlist(fraglist, frag);
            }
	    else if (state & SPACE) {
		fraglist->state = SPACE;
		frag = newelem(SPACE, inp-1, 1, 0);
		appendlist(fraglist, frag);
	    }
// FIXME - something for quoted strings
	}
    }
    return 0;
}
#endif
