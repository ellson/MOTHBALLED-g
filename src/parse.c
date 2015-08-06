#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "emit.h"
#include "parse.h"

static unsigned char unterm, *in;
static char *insp;
static state_t subj, bi, ei;
static elem_t Tree;

static int more_rep(context_t *C, unsigned char prop, state_t ei, state_t bi) {
    if (! (prop & (REP|SREP))) return 0;
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE ) return 0; // no more
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE ) return 1; // more, but no sep needed
    if (prop & SREP) emit_sep(C); // sep needed for SREP sequences
    return 1;
}

static int parse_r(context_t *C, elem_t *root, char *sp,
		       unsigned char prop, int nest, int repc) {
    unsigned char nprop, *frag;
    char *np;
    state_t insi, si, ni, savesubj;
    int rc, len, slen;
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
	if (!in) {
	    if ( !(in = more_in(C)) ) goto done;  // EOF
        }
        insi = char2state[*in]; // get first input state
    }
    else {                     // else continuing state sequence
        if (!in) {             // if we already hit EOF
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

    ei = insi;                 // the char class that ended the last token
    while (insi == WS) {       // eat all leading whitespace
        insi = char2state[*++in];
    }
    while (insi == NLL) {      // end_of_buffer, or end_of_file, during whitespace
	if ( !(in = more_in(C)) ) {
            rc = 1;
	    goto done;  // EOF
        }
        insi = char2state[*in];
        while (insi == WS) {      // eat all remaining leading whitespace
            insi = char2state[*++in];
        }
    }
                               // deal with terminals
    if (si == STRING) {        // string terminals 
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
            append_list(&branch, elem);
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
	    emit_string(C,&branch);
	    rc = 0;
        }
	else {
	    rc = 1;
	}
        bi = insi;                // the char class that terminates the string
        insp = state_machine + (char)insi;
        goto done;
    }
    if (si == insi) {        // single character terminals matching state_machine expectation
        emit_tok(C,si,1,in);

	bi = insi;
        insi = char2state[*++in];
	ei = insi;

        rc = 0;
        insp = state_machine + (char)insi;
        goto done;
    }
    insp = state_machine + (char)insi;

    // else not terminal
    switch (si) {
    case ACT:                     // starting a new ACT
        if (unterm) {             // implicitly terminates preceeding ACT
 	    emit_term(C);
	}
	unterm = 1;               // indicate that this new ACT is unterminated
	break;
    case SUBJECT:
        savesubj = subj;          // push parent's subject
        subj = 0;                 // clear this subject type until known
	break;
    default:
	break;
    }

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

    if (rc == 0) {
        switch (si) {
        case SUBJECT:
            subj = savesubj;      // pop subj
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
            if (nest == 0) {
            switch (si) {
                case ACTIVITY:
                   emit_tree(C, elem);
	           free_list(elem);
	           break;
                default:
	           break;
	        }
            }
	}
    }

    emit_end_state(C, si, rc, nest, repc);

    if (!in) {
        if (unterm) {
 	    emit_term(C);
	}
    }

    return rc;
}

int parse(context_t *C) {
    int rc;

    emit_start_file(C);
    C->size = -1;      // tell more_in() that it is a new stream

    rc = parse_r(C, &Tree, state_machine, SREP, 0, 0);

    emit_end_file(C);
    return rc;
}
