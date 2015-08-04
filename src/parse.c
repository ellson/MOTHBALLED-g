#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "list.h"
#include "inbuf.h"
#include "emit.h"
#include "parse.h"

static unsigned char unterm, *in;
static char *insp, subj, insep;
static elem_t Tree;

static int more_rep(context_t *C, unsigned char *in, unsigned char prop, char bi) {
    char ei;

    if (! (prop & (REP|SREP))) return 0;
    ei = char2state[*(in-1)];
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE ) return 0;
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE ) return 1;
    if (prop & SREP) emit_sep(C);
    return 1;
}

#if 0
static int get_frag (char si) {
    static elem_t leaves;

    insi = char2state[*in++];
    if (insi == WS) {  // eat all leading whitespace
	insep = WS;
        while ( (insi = char2state[*in++]) == WS) {}
    }
    if (insi == NLL) { // end_of_buffer, or end_of_file
        in = more_in(C);
        if (!in) {
	    insep = NLL;
	    return 1;
        }
        insi = char2state[*in++];
        if (insi == WS) {  // eat all remaining leading whitespace
	    insep = WS;
            while ( (insi = char2state[*in++]) == WS) {}
        }
    }
    frag = in-1;
    len = 1;
    ftyp = insi;
    if (si == STRING) {
	while (1) {
	    while (1) {
	        switch (insi) {
	        case ABC: while ( (insi = char2state[*in++]) == ABC) {len++};
			  continue;
	        case UTF: while ( (insi = char2state[*in++]) == UTF) {len++};
			  continue;
	        case AST: while ( (insi = char2state[*in++]) == AST) { /* ignore extra*/ };
			  // FIXME - flag a pattern;
			  continue;
	        }
	        break;
	    }
            emit_frag(C,len,frag);
            elem = new_frag(ftyp,frag,len,NULL);
            append_list(&leaves, elem);
            slen += len;
        }
	if (slen > 0) {
	    emit_string(C,&leaves);
	    rc = 0;
        }
	else {
	    rc = 1;
	}
    }
    else if (si == insi) {
        emit_tok(C,si,1,frag);
	insep = insi;
        insi = char2state[*in++];

        insp = state_machine + insi;
        rc = 0;
        goto done;
    }
}
#endif


static int parse_r(context_t *C, elem_t *root,
		char *sp, unsigned char prop, int nest, int repc) {
    unsigned char nprop, *frag;
    char *np, insi, ftyp, si, ni, savesubj;
    int rc, len, slen;
    elem_t branch;
    elem_t *elem;

    si = sp - state_machine;
    emit_start_state(C, si, prop, nest, repc);

    branch.next = NULL;
    branch.u.list.first = NULL;
    branch.u.list.last = NULL;
    branch.type = LISTELEM;
    branch.len = 0;
    branch.state = si;

    nest++;
    assert (nest >= 0); // catch overflows

    if (insp == NULL) {  // state_machine just started
        insep = WS;      // pretend preceeded by WS
			// to satisfy toplevel SREP or REP
        insi = char2state[*in++]; // get first input state
    }
    else {
	insi = insp - state_machine;
        if (prop & REP) {
            if (insep == WS) { // whitespace not accepted between REP
	        rc = 1;
	        goto done;
	    }
        }
    }
while (1) {
    switch (si) {
    case ACT:              // starting a new ACT
	if (insi == NLL || insep == NLL) {
	    rc = 1;
	    goto done;
        }
        if (unterm) {      // implicitly terminates preceeding ACT
 	    emit_term(C);
	}
	unterm = 1;        // indicate that this ACT is unterminated
	break;
    case SUBJECT:
        savesubj = subj;  // push parent's subject
        subj = 0;         // clear this subject type until known
	break;
    }
    if (insi == WS) {  // eat all leading whitespace
        while ( (insi = char2state[*in++]) == WS) {}
    }
    if (insi == NLL) { // end_of_buffer, or end_of_file
	in = more_in(C);
	if (!in) {
	    rc = 1;
	    goto done;
	}
	insi = char2state[*in++];
        continue;
    }
    break;
}
    insp = state_machine + insi;

    // deal with terminals
    if (si == STRING) { // strings are lists of fragments
        slen = 0;
        insep = insi;
	while(1) {
	    if (insi == NLL) {
		in = more_in(C);
		if (!in) {
		    break;
		}
		insi = char2state[*in++];
		continue;
	    }
            frag = in-1;
            len = 1;
	    ftyp = insi;
            if (insi == ABC) {
                while ( (insi = char2state[*in++]) == ABC) {
		    len++;
                }
            }
            else if (insi == AST) {
                insi = char2state[*in++];
                // FIXME - flag someone that string is a pattern
            }
	    else {
	        break;
	    }
            emit_frag(C,len,frag);
            elem = new_frag(ftyp,frag,len,NULL);
            append_list(&branch, elem);
	    slen += len;
	}
        insp = state_machine + insi;
	if (slen > 0) {
	    emit_string(C,&branch);
	    rc = 0;
        }
	else {
	    rc = 1;
	}
        goto done;
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
	if (nprop & ALT) { // look for ALT
	    if (( rc = parse_r(C, &branch, np,nprop,nest,0)) == 0) {
                break;  // ALT satisfied
	    }
	    // if we fail an ALT then cwcontinue iteration to try next
	} 
	else { // else it is a sequence
	    repc = 0;
	    if (nprop & OPT) { // optional
	        if (( parse_r(C, &branch, np,nprop,nest,repc++)) == 0) {
	            while (more_rep(C, in, nprop, insep)) {
                        if (parse_r(C, &branch, np,nprop,nest,repc++) != 0) break;
		    }
	        }
                rc = 0; // optional can't fail
	    }
	    else { // else not OPTional
	        if (( rc = parse_r(C, &branch, np,nprop,nest,repc++)) != 0) break; 
                // rc is the rc of the first term, which at this point is success
	        while (more_rep(C, in, nprop, insep)) {
                    if (parse_r(C, &branch, np,nprop,nest,repc++) != 0) break;
		}
	    }
	}
	sp++;  // next ALT (if not yet satisfied)  or next sequence item
    }

    if (rc == 0) {
        switch (si) {
        case SUBJECT:
            subj = savesubj; // pop subj
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
	}
    }

done:
    nest--;
    assert (nest >= 0);

    if (rc == 0) {
        elem = list2elem(&branch,0);
        if (si == ACT) {
	    emit_tree(C, elem);
        }
        append_list(root, elem);
    }

    emit_end_state(C, si, rc, nest, repc);
    return rc;
}

int parse(context_t *C) {
    int rc;

    emit_start_file(C);
    C->size = -1;

    in = more_in(C);
    assert(in);

    rc = parse_r(C, &Tree, state_machine,SREP,0,0);

    emit_end_file(C);
    return rc;
}
