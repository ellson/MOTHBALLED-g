#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "emit.h"
#include "grammar.h"
#include "list.h"
#include "parse.h"

static unsigned char unterm, *in, insep, *frag;
static int len;
static char *insp, subj;
static context_t *C;

static int parse_r(char *sp, unsigned char prop, unsigned char nest) {
    unsigned char nprop;
    char *np, insi, si, ni, savesubj;
    int rc;

    si = sp - state_machine;

    emit_start_state(C, si, prop, nest);

    nest++;
    assert (nest >= 0); // catch overflows
        switch (si) {
//        case ACT:
//            if (unterm) {
// 	        emit_term(C);
//	    }
//	    unterm = 1;
//	    // FIXME create new context
//	    break;
        case SUBJECT:
            savesubj = subj; // push subj
	    subj = 0;
	    break;
        }
    switch (si) {
    case ACT:
        if (unterm) {
 	    emit_term(C);
	}
	unterm = 1;
	// FIXME create new context
	break;
    case SUBJECT:
        savesubj = subj;  // might be a nested subject
        subj = 0;
	break;
    }
    if (insp == NULL) {
        insep = WS;
        insi = char2state[*in++];
    }
    else {
	insi = insp - state_machine;
        if (prop & SREP) {
            if (insep == WS) {
                if (insi == WS) {
                    while ( (insi = char2state[*in++]) == WS) {}
		}
                emit_sep(C);
            }
        }
        else if (prop & REP) {
            if (insep == WS) {
	        rc = 1;
	        goto done;
	    }
        }
	else {
            if (si == STRING) {
                if (insep == EQL) {
                    emit_sep(C);
	        }
	    }
	}
    }
    if (insi == WS) {
        while ( (insi = char2state[*in++]) == WS) {}
    }
    if (insi == NLL) { //EOF
        emit_term(C);
    }
    insp = state_machine + insi;

    // deal with terminals
    if (si == STRING) { // STRING terminals
        if (insi == ABC) {
            frag = in-1;
            len = 1;
            while ( (insi = char2state[*in++]) == ABC) {
		len++;
            }
	    emit_frag(C,len,frag);
	    insep = insi;
            insp = state_machine + insi;
	    rc = 0;
            goto done;
        }
        else if (insi == EQL) {
            frag = in-1;
            len = 1;
            insi = char2state[*in++];
	    emit_frag(C,len,frag);
	    insep = insi;
            insp = state_machine + insi;
	    rc = 0;
            goto done;
        }
        else if (insi == AST) {
            frag = in-1;
            len = 1;
            insi = char2state[*in++];
	    emit_frag(C,len,frag);
	    insep = insi;
            insp = state_machine + insi;
	    rc = 0;
            goto done;
        }
    }
    else if (si == insi) { // tokens
        frag = in-1;
        emit_tok(C,si,1,frag);
	insep = insi;
        insi = char2state[*in++];
        insp = state_machine + insi;
        rc = 0;
        goto done;
    }

    rc = 1; // in case no next state is found
    while (( ni = *sp )) { // iterate over ALTs or sequences
        nprop = *PROPP(sp);
        np = sp + ni;
	if ( (nprop & ALT)) { // look for ALT
	    if (( rc = parse_r(np,nprop,nest)) == 0) {
                break;  // ALT satisfied
	    }
	    // if we fail an ALT then cwcontinue iteration to try next
	} 
	else { // else it is a sequence
	    if ( (nprop & OPT)) { // optional
	        if (( parse_r(np,nprop,nest)) == 0) {
	            if ( nprop & (REP|SREP)) {
	                while ( parse_r(np,nprop,nest) == 0) { }
	            }
	        }
                rc = 0; // optional can't fail
	    }
	    else { // else not OPTional
	        if (( rc = parse_r(np,nprop,nest)) != 0) {
		    break; 
                }
                // rc is the rc of the first term, which at this point is success
                if ( nprop & (REP|SREP)) {
	            while ( parse_r(np,nprop,nest) == 0) { }
	        }
	    }
	}
	sp++;  // next ALT (if not yet satisfied)  or next sequence item
    }


    if (rc == 0) {
        switch (si) {
//        case ACT:
//            if (unterm) {
// 	        emit_term(C);
//	    }
//	    unterm = 1;
//	    // FIXME create new context
//	    break;
        case SUBJECT:
            subj = savesubj; // pop subj
	    break;
        }
        ni = np - state_machine;
	switch (ni) {
	case EDGE :
	    if (subj == 0) {
	        subj = EDGE;
//fprintf(OUT," (edge)");
	    }
	    else {
                if (subj == NODE) {
//FIXME	            emit_error(C, "NODE found in EDGE SUBJECT");
	            rc = 1;
		}
	    }
            break;
	case NODE :
	    if (subj == 0) {
	        subj = ni;
//fprintf(OUT," (%s)", ni==NODE?"node":"child");
	    }
	    else {
                if (subj == EDGE) {
	            emit_error(C, "EDGE found in NODE SUBJECT");
	            rc = 1;
		}
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

done:
    nest--;
    assert (nest >= 0);
    emit_end_state(C, si, rc, nest);
    return rc;
}

int parse(unsigned char *input) {
    int rc;

    C = malloc(sizeof(context_t));

    in = input;
    C->nest = 0;
    emit_start_state_machine(C);
    while (( rc = parse_r(state_machine,SREP,0)) == 0) {}
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
