#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "list.h"
#include "parse.h"

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

static char *get_name(char *p) {
    while (*p++) p++;             // traverse to terminator
    return &state_names[(*p<<1)]; // get index for string from the byte after the terminator
}

static void print_edge( char *tail, char *head ) {
    printf("%s %s %s %s", styleLAN, tail, head, styleRAN);
}

static void print_attr ( char attr, char *name, int *cnt ) {
    if (attr) {
	if ((*cnt)++) putc (' ', stdout);
        printf (name);
    }
}

static void print_prop(char prop) {
    int cnt;

    if (prop & (ALT|OPT|SREP|REP|REC)) {
        cnt=0;
        printf("%s", styleLBR);
        putc ('[',stdout);
        print_attr( prop & ALT, "ALT", &cnt);
        print_attr( prop & OPT, "OPT", &cnt);
        print_attr( prop & SREP, "SREP", &cnt);
        print_attr( prop & REP, "REP", &cnt);
        print_attr( prop & REC, "REC", &cnt);
        printf("%s", styleRBR);
    }
}

static void printg_next(char *p, int indent) {
    char *tp, *hp, prop;
    int i, hi;

    tp = p;
    while ((hi = *p++)) {
        prop = *p++;
        hp = p+(hi<<1);

        for (i = indent; i--; ) putc (' ', stdout);
        print_edge(tp, hp);
        print_prop(prop);
        printf("%s", styleLBE);
	if (! (prop & REC)) printg_next(hp, indent+2);
        for (i = indent; i--; ) putc (' ', stdout);
        printf("%s", styleRBE);
    }
}

// recursively walk the grammar - tests all possible transitions
void printg (void) {
    printg_next(&state_machine[ACT<<1], 0);
}

static void print_chars ( char *p ) {
    int i, cnt, si;

    si = (p - state_machine)>>1;
    cnt=0;
    for (i=0; i<0x100; i++) {
        if (si == char2state[i]) {
	    if (cnt++) putc (' ', stdout);
            printf("%02x", i);
	}
    }
}

// just dump the grammar linearly,  should result in same logical graph as printg()
void dumpg (void) {
    int hi;
    char *p, *tp, *hp, prop;

    p = state_machine;
    while (p < (state_machine + sizeof_state_machine)) {
        tp = p;
        if (*p) {
            while ((hi = *p++)) {
                prop = *p++;
                hp = p+(hi<<1);
                print_edge(tp, hp);
                print_prop(prop);
                printf("\n");
	    }
	    p++;
	}
	else {
	    printf("%s%s", get_name(p), styleLBE);
            print_chars(p);
	    printf("%s", styleRBE);
	    p+=2;	
	}
    }
}

static unsigned char *inp, c;
static int in;

#if 0
static int parse_next(int s) {
    int *pn, n, rc;

#if 1
    int ss;
    char *s_name;
#endif
#if 2
    char *in_name;
#endif

#if 1
    ss = s & 0xFF;
    s_name = *state_name[ss >> 1];
    printf("%s\n", s_name);
#endif

    if ( in == ss ) {
#if 2
        in_name = *state_name[in >> 1];
	printf("    %s\n", in_name);
#endif
        c = *inp++;
        in = char2state[c];
	return 0;
    }
    rc = 1;
    pn = state_next[ss >> 1];
    while ((n = *pn++)) {
//        if (n & REC) continue;     // FIXME - maybe allocate new inp buffers?

        if ((n ^ s) & TWO) continue;

	if      ( (n & ALT)) {    if (( rc = parse_next(n) ) != 0) continue;} 
	else if (!(n & OPT)) {    if (( rc = parse_next(n) ) != 0) break; } 
	if      ( (n & REP)) { while (( rc = parse_next(n) ) == 0); break; }
    }
    return rc;
}
#endif

static int parse_next(char *p) {
    char *tp, *hp, prop;
    int i, hi;

    tp = p;
    while ((hi = *p++)) {
        prop = *p++;
        hp = p+(hi<<1);

        print_edge(tp, hp);
        print_prop(prop);
        printf(" {\n");
//	if (! (prop & REC)) printg_next(hp, indent+2);
        printf("}\n");
    }
    return 1;
}

int parse(unsigned char *in) {
    c = *in++;
    inp = &state_machine[char2state[c]<<1];
    return parse_next(&state_machine[ACT<<1]);
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
