#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "list.h"
#include "emit.h"
#include "parse.h"

static unsigned char unterm, *in;
static char *insp, subj, insep;

static int more(context_t *C, unsigned char *in, unsigned char prop, char bi) {
    char ei;

    if (! (prop & (REP|SREP))) return 0;
    ei = char2state[*(in-1)];
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE ) return 0;
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE ) return 1;
    if (prop & SREP) emit_sep(C);
    return 1;
}

static int parse_r(context_t *C, elem_t *root,
		char *sp, unsigned char prop, int nest, int repc) {
    unsigned char nprop, *frag;
    char *np, insi, ftyp, si, ni, savesubj;
    int rc, len, slen;
    elem_t branch;
    elem_t *elem;

    si = sp - state_machine;
    emit_start_state(C, si, prop, nest, repc);

    elem = NULL;
    branch.next = NULL;
    branch.u.list.first = NULL;
    branch.u.list.last = NULL;
    branch.type = LISTELEM;
    branch.len = 0;
    branch.state = si;

    nest++;
    assert (nest >= 0); // catch overflows
    if (insp == NULL) {  // state_nschine just started
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
    if (insi == NLL) { //EOF
	rc = 1;
	goto done;
    }
    insp = state_machine + insi;

    // deal with terminals
    if (si == STRING) { // strinds 
        slen = 0;
        insep = insi;
	while(1) {
            frag = in-1;
            len = 1;
	    ftyp = insi;
            if (insi == ABC) {
                while ( (insi = char2state[*in++]) == ABC) {
		    len++;
                }
            }
            else if (insi == EQL || insi == AST) {
                insi = char2state[*in++];
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
	    elem = list2elem(&branch,slen);
	    emit_string(C,elem);
	    rc = 0;
        }
	else {
	    elem = NULL;
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
	            while (more(C, in, nprop, insep)) {
                        if (parse_r(C, &branch, np,nprop,nest,repc++) != 0) break;
		    }
	        }
                rc = 0; // optional can't fail
	    }
	    else { // else not OPTional
	        if (( rc = parse_r(C, &branch, np,nprop,nest,repc++)) != 0) break; 
                // rc is the rc of the first term, which at this point is success
	        while (more(C, in, nprop, insep)) {
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

int parse(context_t *C, unsigned char *input) {
    int rc;

    in = input;
    emit_start_state_machine(C);
    rc = parse_r(C, &(C->Tree), state_machine,SREP,0,0);
    emit_end_state_machine(C);
    return rc;
}
