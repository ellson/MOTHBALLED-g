#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "stats.h"
#include "emit.h"
#include "parse.h"
#include "tokens.h"

static unsigned char unterm;
static char *insp;
static state_t subj, bi, ei;
static elem_t *sameend_elem;

static success_t more_rep(context_t *C, unsigned char prop, state_t ei, state_t bi) {
    if (! (prop & (REP|SREP))) return FAIL;
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE ) return FAIL; // no more
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE ||
        ei == LPN || ei == LAN || ei == LBR || ei == LBE) return SUCCESS; // more, but no sep needed
    if (prop & SREP) emit_sep(C); // sep needed for SREP sequences
    return SUCCESS;
}

static success_t parse_r(context_t *C, elem_t *root, char *sp,
		       unsigned char prop, int nest, int repc) {
    unsigned char nprop;
    char *np;
    state_t si, ni, savesubj;
    success_t rc;
    elem_t *elem;
    elem_t branch = {
	.next = NULL,
	.u.list.first = NULL,
	.u.list.last = NULL,
	.v.list.refs = 0,
	.type = LISTELEM,
	.state = 0
    };

    rc = SUCCESS;
    si = (state_t)(sp - state_machine);
    emit_start_state(C, si, prop, nest, repc);
    branch.state = si;

    nest++;
    assert (nest >= 0);        // catch overflows

    if (! C->inbuf) {          // state_machine just started
        bi = WS;               // pretend preceeded by WS
			       // to satisfy toplevel SREP or REP
			       // (Note, first REP of a REP sequence *can* be preceeded by WS,
			       //      just not the rest of the REPs. )
        C->insi = NLL;;        // pretend last input was a terminating NLL
    }
    else {                     // else continuing state sequence
        if (prop & REP) {      // if the sequence is a non-space REPetition
            if (bi == WS) {    // whitespace not accepted between REP
// FIXME - Error message here ?
	        rc = FAIL;     // so the REP is terminated
	        goto done;
	    }
        }
    }

    // deal with terminal states: whitespace, string, token
    
    ei = C->insi;              // the char class that ended the last token
    if ( (rc = parse_whitespace(C)) ) {
	goto done;             // EOF during whitespace
    }
 
    if (si == STRING) {        // string terminals 

        rc = parse_string(C, &branch);
        bi = C->insi;             // the char class that terminates the string

        insp = state_machine + (char)(C->insi);
        goto done;
    }
    if (si == C->insi) {          // single character terminals matching state_machine expectation

	bi = C->insi;
        rc = parse_token(C);
	ei = C->insi;

        insp = state_machine + (char)(C->insi);

// FIXME - set flags for '~' and '='
        goto done;
    }
    insp = state_machine + (char)(C->insi);


    // else non terminal state -- state entry processing
 
    switch (si) {
    case ACT:
        if (unterm) {             // implicitly terminates preceeding ACT
 	    emit_term(C);
	}
	unterm = 1;               // indicate that this new ACT is unterminated
	break;
    case SUBJECT:
        // FIXME - don't stack ... use context for this
        savesubj = subj;          // push parent's subject
        subj = 0;                 // clear this subject type until known
	break;
    default:
	break;
    }

    // parse next state
 
    rc = FAIL;                    // init rc to "fail" in case no next state is found
    while (( ni = (state_t)(*sp) )) {        // iterate over ALTs or sequences
        nprop = *PROPP(sp);
        np = sp + (char)ni;
	if (nprop & ALT) {        // look for ALT
	    if (( rc = parse_r(C, &branch, np,nprop,nest,0)) == SUCCESS) {
                break;            // ALT satisfied
	    }
	                          // we failed an ALT so continue iteration to try next ALT
	} 
	else {                    // else it is a sequence
	    repc = 0;
	    if (nprop & OPT) {    // optional
	        if (( parse_r(C, &branch, np,nprop,nest,repc++)) == SUCCESS) {
	            while (more_rep(C, nprop, ei, bi) == SUCCESS) {
                        if (parse_r(C, &branch, np,nprop,nest,repc++) == FAIL) {
			    break;
			}
		    }
	        }
                rc = SUCCESS;           // optional can't fail
	    }
	    else {                // else not OPTional
	        if (( rc = parse_r(C, &branch, np,nprop,nest,repc++)) == FAIL) {
		    break; 
		}
                // rc is from the first term in the non-optional sequence,
                // which at this point is success
	        while (more_rep(C, nprop, ei, bi) == SUCCESS) {
                    if (parse_r(C, &branch, np,nprop,nest,repc++) == FAIL) {
			break;
		    }
		}
	    }
	}
	sp++;                     // next ALT (if not yet satisfied) 
			          // or next sequence item
    }

    // Any subtree rewrites or emit before adding branch to root in the state exit processing
    if (rc == SUCCESS) {
        switch (si) {
        case LEG:
#if 1
	    if (bi == EQL) {
                if (! sameend_elem) {
	            emit_error(C, "No prior LEG found for sameend substitution");
	            rc = FAIL;
	        }
//		elem = ref_list(si, elem);

                elem = ref_list(si, sameend_elem);
// FIXME can be multiple ENDPOINTS in a LEG, need a while here
//                append_list(&branch, sameend_elem->u.list.first);
            }
            if (sameend_elem) {
	        sameend_elem = sameend_elem -> next;
            }
#endif
	    break;
        case SUBJECT:
            subj = savesubj;      // pop subj     // FIXME

            elem = ref_list(si, &branch);
            push_list(&(C->subject), elem);  // save the subject of this act at this level of containment

#if 0
putc ('\n', stdout);
print_list(stdout, &(C->subject), 0, ' ');
putc ('\n', stdout);
#endif
#if 1
            // update samends
            //    -- free old samends
	    free_list(&(C->sameend_legs));
            //    -- replace with new ones
            elem = move_list(si, &(C->sameend_legs_new));
	    append_list(&(C->sameend_legs), elem);
            // initial iterator io point to first samend
	    sameend_elem = C->sameend_legs.u.list.first;
#endif
	    break;
	case ACT:
            stat_actcount++;
            pop_list(&(C->subject));  // discard the subject of this act at this level of containment
#if 1
            emit_tree(C, &branch);
// FIXME - at the moment this is freeing an active input_buffer
//            free_list(&branch);
#endif
            break;
        default:
	    break;
	}
    }

    // State exit processing

done:
    if (rc == SUCCESS && branch.u.list.first != NULL) { // ignore fails and empty lists
        elem = move_list(si, &branch);
        append_list(root, elem);
        switch (si) {
//putc ('\n', stdout);
//print_list(stdout, elem, 0, ' ');
//putc ('\n', stdout);
//	        free_list(&sameend_legs);      // free old sameend list
//		elem = ref_list(si, root->u.list.last);  // replace with new list, fully substituted.

//                append_list(&(C->sameend_legs_new), elem);

// putc ('\n', stdout);
// print_list(stdout, &new_sameend_legs, 0, ' ');
// putc ('\n', stdout);
	case EDGE :
	    if (subj == 0) {
	        subj = EDGE;
	    }
	    else {
                if (subj == NODE) {
	            emit_error(C, "NODE found in EDGE SUBJECT");
	            rc = FAIL;
		}
	    }
            break;
	case NODE :
	    if (subj == 0) {
	        subj = NODE;
	    }
	    else {
                if (subj == EDGE) {
	            emit_error(C, "EDGE found in NODE SUBJECT");
	            rc = FAIL;
		}
	    }
            break;
        case TERM :   
            if (unterm) {
 	        emit_term(C);
	    }
	    unterm = 0;
	    break;
        case CONTAINER :
	    stat_containercount++;
            if (unterm) {
 	        emit_term(C);
	    }
	    unterm = 0;
	    break;
        default:
	    break;
	}
    }

    nest--;
    assert (nest >= 0);
    emit_end_state(C, si, rc, nest, repc);

    return rc;
}

success_t parse(context_t *C) {
    success_t rc;
    elem_t root = {
        .next = NULL,
        .u.list.first = NULL,
        .u.list.last = NULL,
        .v.list.refs = 0,
        .type = LISTELEM,
        .state = 0
    };

    emit_start_file(C);
    rc = parse_r(C, &root, state_machine, SREP, 0, 0);
    if (! (C->in)) {           // if at EOF
        if (unterm) {
 	    emit_term(C);      // EOF is an implicit terminator
	}
        unterm = 0;
    }
    emit_end_file(C);
    return rc;
}
