#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "context.h"
#include "stats.h"
#include "emit.h"
#include "parse.h"
#include "token.h"

static unsigned char unterm;
static state_t subj, bi, ei;
static elem_t *sameend_elem;

static success_t more_rep(context_t *C, unsigned char prop, state_t ei, state_t bi) {
    if (! (prop & (REP|SREP))) return FAIL;
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE ) {
        return FAIL;           // no more repetitions
    }
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE || (ei != ABC && ei != AST)) {
        return SUCCESS;        // more repetitions, but additional WS sep is optional
    }
    if (prop & SREP) {
        emit_sep(C);           // sep is non-optional, emit the minimal sep
    }
    return SUCCESS;            // more repetitions
}

static success_t parse_activity(context_t *C); // forward declaration for recursion

static success_t parse_r(context_t *C, elem_t *root, state_t si, unsigned char prop, int nest, int repc) {
    unsigned char nprop;
    char so;                  // offset to next state, signed
    state_t ti, ni, savesubj;
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
    static unsigned char nullstring[] = {'\0'};

    rc = SUCCESS;
    emit_start_state(C, si, prop, nest, repc);
    branch.state = si;

    nest++;
    assert (nest >= 0);        // catch overflows

    if (! C->inbuf) {          // state_machine just started
        bi = WS;               // pretend preceeded by WS
			       // to satisfy toplevel SREP or REP
			       // (Note, first REP of a REP sequence *can* be preceeded by WS,
		               //      just not the rest of the REPs. )
        C->in = nullstring;    // fake it;
        C->insi = NLL;         // pretend last input was a terminating NLL
    }

    // deal with terminal states: whitespace, string, token
    
    ei = C->insi;              // the char class that ended the last token
    if ( (rc = parse_whitespace(C)) == FAIL ) {
	goto done;             // EOF during whitespace
    }
    if (si == C->insi) {       // single character terminals matching state_machine expectation
	bi = C->insi;
        rc = parse_token(C);
	ei = C->insi;
// FIXME - set flag for '~'
        goto done;
    }
    switch (si) {
    case ACTIVITY:
	if (bi == LBE) { // if not top-level of containment
//fprintf(OUT,"\nactivity ei=%d bi=%d si=%d insi=%d in=%s\n", ei, bi, si, C->insi, C->in);
#if 1
            bi = NLL;
            rc = parse_activity(C);// recursively process contained ACTIVITY in to its own root
            bi = C->insi;          // the char class that terminates the ACTIVITY
        goto done;
#endif
}
	break;
    case STRING:
        rc = parse_string(C, &branch);
        bi = C->insi;          // the char class that terminates the STRING
        goto done;
	break;
    case ACT:
// FIXME - unterm needs to be stacked
        if (unterm) {             // implicitly terminates preceeding ACT
 	    emit_term(C);
	}
	unterm = 1;               // indicate that this new ACT is unterminated
	break;
    case SUBJECT:
// FIXME - savesubj needs to be ... use context for this
        savesubj = subj;          // push parent's subject
        subj = 0;                 // clear this subject type until known
	break;
    case CONTAINER:
        C->containment++;
	break;
    default:
	break;
    }

    // Use state_machine[si] to determine next state
 
    rc = FAIL;                    // init rc to FAIL in case no ALT is satisfied
    ti = si;
    while (( so = state_machine[ti] )) { // iterate over ALTs or sequences
        nprop = state_props[ti];   // get the props for the transition from the current state (OPT, ALT, REP etc)
	                          // at this point, ni is a signed, non-zero offset to the next state
        ni = ti + so;             // we get to the next state by adding the offset to the current state.
	if (nprop & ALT) {        // look for ALT
	    if (( rc = parse_r(C, &branch, ni, nprop, nest, 0)) == SUCCESS) {
                break;            // ALT satisfied
	    }
	                          // we failed an ALT so continue iteration to try next ALT
	} 
	else {                    // else it is a sequence
	    repc = 0;
	    if (nprop & OPT) {    // optional
	        if (( parse_r(C, &branch, ni, nprop, nest, repc++)) == SUCCESS) {
	            while (more_rep(C, nprop, ei, bi) == SUCCESS) {
                        if (parse_r(C, &branch, ni, nprop, nest, repc++) == FAIL) {
			    break;
			}
		    }
	        }
	    }
	    else {                // else not OPTional
	        if (( rc = parse_r(C, &branch, ni, nprop, nest, repc++)) == FAIL) {
		    break; 
		}
                // A 1-or-more repetition is successful is the first one was a success
	        while (more_rep(C, nprop, ei, bi) == SUCCESS) {
                    if (( rc = parse_r(C, &branch, ni, nprop, nest, repc++)) == FAIL) {
			break;
		    }
		}
	    }
	}
	ti++;                     // next ALT (if not yet satisfied), or next sequence item
    }

    // Any subtree rewrites or emit before adding branch to root in the state exit processing
    if (rc == SUCCESS) {
        switch (si) {
        case LEG:
#if 1
	    if (bi == EQL) {
                if (! sameend_elem) {
	            emit_error(C, si, "No prior LEG found for sameend substitution in");
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
        case CONTAINER:
            C->containment--;
	    break;
        case SUBJECT:
            emit_subject(C, &branch);

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
            emit_act(C, &branch);

            pop_list(&(C->subject));  // discard the subject of this act at this level of containment
#if 0
// FIXME - at the moment this is freeing an active input_buffer
            free_list(&branch);
#endif
            break;
        default:
	    break;
	}
    }

    // State exit processing

done:
    if (rc == SUCCESS) {             
        if (branch.u.list.first != NULL) { // ignore empty lists
            elem = move_list(si, &branch);
            append_list(root, elem);
            switch (si) {
    //putc ('\n', stdout);
    //print_list(stdout, elem, 0, ' ');
    //putc ('\n', stdout);
    //    free_list(&sameend_legs);      // free old sameend list
    //    elem = ref_list(si, root->u.list.last);  // replace with new list, fully substituted.
    
    //    append_list(&(C->sameend_legs_new), elem);
    
    // putc ('\n', stdout);
    // print_list(stdout, &new_sameend_legs, 0, ' ');
    // putc ('\n', stdout);
            case EDGE :
                if (subj == 0) {
                    subj = EDGE;
                }
                else {
                    if (subj == NODE) {
                        emit_error(C, si, "NODE subject includes");
                    }
                }
                break;
            case NODE :
                if (subj == 0) {
                    subj = NODE;
                }
                else {
                    if (subj == EDGE) {
                        emit_error(C, si, "EDGE subject includes");
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
    }

    nest--;
    assert (nest >= 0);
    emit_end_state(C, si, rc, nest, repc);

    return rc;
}

static success_t parse_activity(context_t *C) {
    success_t rc;
    elem_t root = {         // the output parse tree
        .next = NULL,
        .u.list.first = NULL,
        .u.list.last = NULL,
        .v.list.refs = 0,
        .type = LISTELEM,
        .state = 0
    };

    emit_start_activity(C);

    if ((rc = parse_r(C, &root, ACTIVITY, SREP, 0, 0)) != SUCCESS) {
#if 1
        if (C->insi == NLL) { // EOF is OK
            rc = SUCCESS;
        }
        else {
// FIXME - Show details: filename, line#, char#, badchar 
// FIXME - Keep track of: line#, char#
            emit_error(C, ACTIVITY, "Parse error");
        }
#endif
    }

    if (unterm) {
 	emit_term(C);      // EOF is an implicit terminator
    }
    unterm = 0;

    emit_end_activity(C);
    return rc;
}

success_t parse(int *pargc, char *argv[]) {
    success_t rc;
    context_t context = {   // the input context (stacked with each CONTAINER recursion)
	.pargc = pargc,
	.argv = argv,
	.filename=NULL,
	.file = NULL,
	.inbuf = NULL,
	.in = NULL,
	.insi = CONTAINER,
    };

    rc = parse_activity(&context);

    return rc;
}
