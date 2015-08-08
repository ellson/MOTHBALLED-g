#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "emit.h"
#include "parse.h"

static unsigned char unterm;
static char *insp;
static state_t subj, bi, ei;
static elem_t *sameend_elem;
static elem_t sameend_legs = {
	.next = NULL,
	.u.list.first = NULL,
	.u.list.last = NULL,
	.v.list.refs = 0,
	.type = LISTELEM,
	.state = 0
};
static elem_t new_sameend_legs = {
	.next = NULL,
	.u.list.first = NULL,
	.u.list.last = NULL,
	.v.list.refs = 0,
	.type = LISTELEM,
	.state = 0
};

static int more_rep(context_t *C, unsigned char prop, state_t ei, state_t bi) {
    if (! (prop & (REP|SREP))) return 0;
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE ) return 0; // no more
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE ||
        ei == LPN || ei == LAN || ei == LBR || ei == LBE) return 1; // more, but no sep needed
    if (prop & SREP) emit_sep(C); // sep needed for SREP sequences
    return 1;
}

// consume all whitespace up to next token, or EOF
// FIXME - this function is the place to deal with comments
static int parse_whitespace(context_t *C, state_t *pinsi) {
    state_t insi;
    unsigned char *in;
    int rc;

    in = C->in;
    insi = *pinsi;
    rc = 0;
    while (insi == WS) {       // eat all leading whitespace
        insi = char2state[*++in];
    }
    while (insi == NLL) {      // end_of_buffer, or EOF, during whitespace
	if ( !(in = more_in(C)) ) {
	    rc = 1;            // EOF
	    break;
        }
        insi = char2state[*in];
        while (insi == WS) {      // eat all remaining leading whitespace
            insi = char2state[*++in];
        }
    }
    *pinsi = insi;
    C->in = in;
    return rc;
}

// collect fragments to form a STRING token
// FIXME - this function is the place to deal with quoting and escaping
static int parse_string(context_t *C, state_t *pinsi, elem_t *fraglist) {
    int rc, slen, len;
    unsigned char *frag, *in;
    state_t insi;
    elem_t *elem;

    in = C->in;
    insi = *pinsi;
    slen = 0;
    while (1) {
        if (insi != ABC && insi != UTF) {
            if (insi == AST) {
                // FIXME - flag a pattern;
            }
            else {
                break;
            }
        }
        frag = in;
        len = 1;
        insi = char2state[*++in];
        while (1) {
            if (insi == ABC) {
                len++;
                while ( (insi = char2state[*++in]) == ABC) {len++;}
                continue;
            }
            if (insi == UTF) {
                len++;
                while ( (insi = char2state[*++in]) == UTF) {len++;}
                continue;
            }
            if (insi == AST) {
                len++;
                // FIXME - flag a pattern;
                while ( (insi = char2state[*++in]) == AST) {len++;}
                continue;
            }
            break;
        }
        emit_frag(C,len,frag);
        elem = new_frag(ABC,frag,len,C->inbuf);
        append_list(fraglist, elem);
        slen += len;

        if (insi != NLL) {
            break;
        }
        // end_of_buffer during, or end_of_file terminating, string
        if ( ! (in = more_in(C)) ) break;   // EOF
        // else continue with next buffer
        insi = char2state[*in];
    }
    if (slen > 0) {
        emit_string(C,fraglist);
        rc = 0;
    }
    else {
        rc = 1;
    }
    *pinsi = insi;
    C->in = in;
    return rc;
}

// process single character tokens
static int parse_token(context_t *C, state_t *pinsi) {
    emit_tok(C, *pinsi, 1, C->in);
    *pinsi = char2state[*++(C->in)];
    return 0;
}

static int parse_r(context_t *C, elem_t *root, char *sp,
		       unsigned char prop, int nest, int repc) {
    unsigned char nprop;
    char *np;
    state_t insi, si, ni, savesubj;
    int rc;
    elem_t *elem;
    elem_t branch = {
	.next = NULL,
	.u.list.first = NULL,
	.u.list.last = NULL,
	.v.list.refs = 0,
	.type = LISTELEM,
	.state = 0
    };

    rc = 0;
    si = (state_t)(sp - state_machine);
    emit_start_state(C, si, prop, nest, repc);
    branch.state = si;

    nest++;
    assert (nest >= 0);        // catch overflows

    if (insp == NULL) {        // state_machine just started
        bi = WS;               // pretend preceeded by WS
			       // to satisfy toplevel SREP or REP
			       // (Note, first REP of a REP sequence *can* be preceeded by WS,
			       //      just not the rest of the REPs. )
	if (!(C->in)) {
	    if ( !(C->in = more_in(C)) ) goto done;  // EOF
        }
        insi = char2state[*C->in]; // get first input state
    }
    else {                     // else continuing state sequence
        if (!(C->in)) {        // if we already hit EOF
            rc = 1;            // then there is nothing to be done
            goto done;
        }
	insi = (state_t)(insp - state_machine);
        if (prop & REP) {      // if the sequence is a non-space REPetition
            if (bi == WS) {    // whitespace not accepted between REP
	        rc = 1;        // so the REP is terminated
	        goto done;
	    }
        }
    }


    // deal with terminal states: whitespace, string, token
    
    ei = insi;                 // the char class that ended the last token
    if ( (rc = parse_whitespace(C, &insi)) ) {
	goto done;             // EOF during whitespace
    }
 
    if (si == STRING) {        // string terminals 

        rc = parse_string(C, &insi, &branch);
        bi = insi;             // the char class that terminates the string

        insp = state_machine + (char)insi;
        goto done;
    }
    if (si == insi) {          // single character terminals matching state_machine expectation

	bi = insi;
        rc = parse_token(C, &insi);
	ei = insi;

        insp = state_machine + (char)insi;

// FIXME - set flags for '~' and '='
        goto done;
    }
    insp = state_machine + (char)insi;


    // else non terminal state -- some state entry processing
 
    switch (si) {
    case ACT:                     // starting a new ACT
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
 
    rc = 1;                       // init rc to "fail" in case no next state is found
    while (( ni = (state_t)(*sp) )) {        // iterate over ALTs or sequences
        nprop = *PROPP(sp);
        np = sp + (char)ni;
	if (nprop & ALT) {        // look for ALT
	    if (( rc = parse_r(C, &branch, np,nprop,nest,0)) == 0) {
                break;            // ALT satisfied
	    }
	                          // we failed an ALT so continue iteration to try next ALT
	} 
	else {                    // else it is a sequence
	    repc = 0;
	    if (nprop & OPT) {    // optional
	        if (( parse_r(C, &branch, np,nprop,nest,repc++)) == 0) {
	            while (more_rep(C, nprop, ei, bi)) {
                        if (parse_r(C, &branch, np,nprop,nest,repc++) != 0) break;
		    }
	        }
                rc = 0;           // optional can't fail
	    }
	    else {                // else not OPTional
	        if (( rc = parse_r(C, &branch, np,nprop,nest,repc++)) != 0) break; 
                                  // rc is the rc of the first term,
                                  // which at this point is success
	        while (more_rep(C, nprop, ei, bi)) {
                    if (parse_r(C, &branch, np,nprop,nest,repc++) != 0) break;
		}
	    }
	}
	sp++;                     // next ALT (if not yet satisfied) 
			          // or next sequence item
    }

    // some state exit processing

    if (rc == 0) {
        switch (si) {
        case ACT:
            actcount++;
	    break;
        case SUBJECT:
            subj = savesubj;      // pop subj
	    free_list(&sameend_legs);
            sameend_legs = new_sameend_legs;

// FIXME - more concise method of clearing
new_sameend_legs.next = NULL;
new_sameend_legs.u.list.first = NULL;
new_sameend_legs.u.list.last = NULL;
new_sameend_legs.v.list.refs = 0;
new_sameend_legs.type = LISTELEM;
new_sameend_legs.state = 0;

//putc ('\n', stdout);
//print_list(stdout, &sameend_legs, 0, ' ');
//putc ('\n', stdout);
	    sameend_elem = sameend_legs.u.list.first;
//putc ('\n', stdout);
//print_list(stdout, sameend_elem, 0, ' ');
//putc ('\n', stdout);
	    break;
	case LEG :
//fprintf(stdout,"\nei=%d bi=%d",ei, bi);
	    if (bi == EQL) {
                if (! sameend_elem) {
	            emit_error(C, "No prior LEG found for sameend substitution");
	            rc = 1;
	        }
//              elem = ref_list(si, sameend_elem);
// FIXME can be multiple ENDPOINTS in a LEG, need a while here
                append_list(&branch, sameend_elem->u.list.first);
            }
            if (sameend_elem) {
	        sameend_elem = sameend_elem -> next;
            }
	    break;
	case EDGE :
	    if (subj == 0) {
	        subj = EDGE;
	    }
	    else {
                if (subj == NODE) {
	            emit_error(C, "NODE found in EDGE SUBJECT");
	            rc = 1;
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
        default:
	    break;
	}
    }

    
done:
    nest--;
    assert (nest >= 0);

    if (rc == 0) {
        if (branch.u.list.first) { // ignore empty lists
            elem = move_list(si, &branch);
            append_list(root, elem);
// FIXME - do containers in a separate context
            if (nest == 0 && si == ACTIVITY) {
                emit_tree(C, elem);
	        free_list(elem);
            }
            if (si == LEG) {
// FIXME -- good to here,  but the '=' has already been emitted.
//putc ('\n', stdout);
//print_list(stdout, elem, 0, ' ');
//putc ('\n', stdout);
//	        free_list(&sameend_legs);      // free old sameend list
//		elem = ref_list(si, root->u.list.last);  // replace with new list, fully substituted.
                append_list(&new_sameend_legs, elem);
                

// putc ('\n', stdout);
// print_list(stdout, &new_sameend_legs, 0, ' ');
// putc ('\n', stdout);
            }
	}
    }

    emit_end_state(C, si, rc, nest, repc);

    if (!(C->in)) {                      // if at EOF
        if (unterm) {
 	    emit_term(C);           // EOF is an implicit terminator
	}
        // this does not change previously set rc
    }

    return rc;
}

int parse(context_t *C) {
    int rc;
    elem_t root = {
        .next = NULL,
        .u.list.first = NULL,
        .u.list.last = NULL,
        .v.list.refs = 0,
        .type = LISTELEM,
        .state = 0
    };

    emit_start_file(C);
    C->size = -1;      // tell more_in() that it is a new stream

    rc = parse_r(C, &root, state_machine, SREP, 0, 0);

    emit_end_file(C);
    return rc;
}
