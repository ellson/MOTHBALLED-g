#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "emit.h"
#include "grammar.h"
#include "list.h"
#include "parse.h"

static unsigned char c, unterm, sep, *in, *frag;
static int flen;
static char *insp, subj;
static context_t *C;

static int parse_r(char *sp) {
    unsigned char prop;
    char *np, insi, si, nxt;
    int rc;

    C->nest++;
    si = sp - state_machine;
    switch (si) {
    case ACT:
        if (unterm) {
 	    emit_term(C);
	}
	unterm = 1;
	// FIXME create new context
	break;
    case SUBJECT:
	// FIXME need to be in context
        subj = 0;
	break;
    }
    emit_start_state(C, sp);
    if (insp == NULL) {
        c = *in++;
        insi = char2state[c];
    }
    else {
	insi = insp - state_machine;
    }
    while (insi == WS) {
        c = *in++;
        insi = char2state[c];
    }
    if (insi == NLL) { //EOF
        emit_term(C);
    }
    insp = state_machine + insi;
    frag = in-1;
    flen = 0;

    rc = 1;
    if (sp == state_machine + STRING) {
        if (insi == ABC) {
            while  ( insi == ABC) {
                c = *in++;
		insi = char2state[c];
	        flen++;
            }
            rc = 0;
	    emit_string(C, frag, flen);
            insp = state_machine + insi;
            frag = in-1;
            flen=1;
        }
    }
    else if (sp == insp) {
	insp = NULL;
        rc = 0;
        emit_token(C, c);
    }

    if (rc == 1) {
        while (( nxt = *sp )) { // iterate over sequeces or ALT sets
            prop = *PROPP(sp);

	    emit_indent(C);
	    emit_prop(C,prop);

            np = sp + nxt;

	    if ( (prop & ALT)) { // if we fail an ALT then try next
	        if (( rc = parse_r(np)) != 0) {
                    sp++;
		    continue;
	        }
                break;
	    } 
	    if ( (prop & OPT)) {  // optional (can't fail)
	        if (( parse_r(np)) == 0) {
	            if (( sep = (prop & (REP|SREP)) )) {
	                while (( parse_r(np)) == 0) { }
	            }
	        }
                rc = 0;
	    }
	    else { // else not OPTional
	        if (( rc = parse_r(np)) != 0) {
		    break;
                }
	        if (( sep = (prop & (REP|SREP)) )) {
	            while (( parse_r(np)) == 0) { }
	        }
	    }
	    sp++;
        }
    }

    if (rc == 0) {
        si = np - state_machine;
	switch (si) {
	case EDGE :
	    if (subj == 0) {
	        subj = EDGE;
	    }
	    else {
	        emit_error(C, "EDGE found in NODE SUBJECT");
	        rc = 1;
	    }
            break;
	case NODE :
	    if (subj == 0) {
	        subj = NODE;
	    }
	    else {
	        emit_error(C, "NODE found in EDGE SUBJECT");
	        rc = 1;
	    }
            break;
        case TERM :
        case CONTAINER :
            if (unterm) {
 	        emit_term(C);
	    }
	    unterm = 0;
	    break;
	}
    }

    C->nest--;
    emit_indent(C);
    emit_end_state(C, rc);
    return rc;
}

int parse(unsigned char *input) {
    int rc;

    C = malloc(sizeof(context_t));

    in = input;
    C->nest = 0;
    emit_start_state_machine(C);
    while (( rc = parse_r(state_machine)) == 0) {}
    emit_end_state_machine(C);
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
