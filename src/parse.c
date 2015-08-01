#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "list.h"
#include "emit.h"
#include "parse.h"

static context_t Context;
static elem_t Tree;
static elem_t Leaves;

static unsigned char unterm, *in, *frag;
static int len, slen;
static char *insp, subj, insep;
static context_t *C;
static elem_t *elem, *branch, *leaves;

static int more(unsigned char *in, unsigned char prop, char bi) {
    char ei;

    if (! (prop & (REP|SREP))) return 0;
    ei = char2state[*(in-1)];
    if (ei == RPN || ei == RAN || ei == RBR || ei == RBE ) return 0;
    if (bi == RPN || bi == RAN || bi == RBR || bi == RBE ) return 1;
    if (prop & SREP) emit_sep(C);
    return 1;
}

static int parse_r(char *sp, unsigned char prop, int nest, int repc) {
    unsigned char nprop;
    char *np, insi, ftyp, si, ni, savesubj;
    int rc;
    elem_t *parent_branch;

    si = sp - state_machine;

    emit_start_state(C, si, prop, nest, repc);

    parent_branch = branch;  // save state of caller's result list
    branch = new_list(si);   // create new list for my result

    nest++;
    assert (nest >= 0); // catch overflows
    switch (si) {
    case ACT:              // starting a new ACT
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
    if (insi == WS) {  // eat all leading whitespace
        while ( (insi = char2state[*in++]) == WS) {}
    }
    if (insi == NLL) { //EOF
        emit_term(C);
	rc = 1;
	goto done;
    }
    insp = state_machine + insi;

    // deal with terminals
    if (si == STRING) { // strinds 
        leaves = &Leaves;
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
            elem = new_frag(ftyp,frag,len,0);
            append_list(leaves, elem);
	    slen += len;
	}
        insp = state_machine + insi;
	if (slen > 0) {
	    elem = list2elem(leaves,slen);
	    append_list(branch, elem);
            emit_string(C,branch);
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
	    if (( rc = parse_r(np,nprop,nest,0)) == 0) {
                break;  // ALT satisfied
	    }
	    // if we fail an ALT then cwcontinue iteration to try next
	} 
	else { // else it is a sequence
	    repc = 0;
	    if (nprop & OPT) { // optional
	        if (( parse_r(np,nprop,nest,repc++)) == 0) {
	            while (more(in, nprop, insep)) {
                        if (parse_r(np,nprop,nest,repc++) != 0) break;
		    }
	        }
                rc = 0; // optional can't fail
	    }
	    else { // else not OPTional
	        if (( rc = parse_r(np,nprop,nest,repc++)) != 0) break; 
                // rc is the rc of the first term, which at this point is success
	        while (more(in, nprop, insep)) {
                    if (parse_r(np,nprop,nest,repc++) != 0) break;
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

    elem = list2elem(branch, repc);
    branch = parent_branch;
    if (rc) {
	free_list(elem);
    }
    else {
	append_list(branch, elem);
	if (si == ACT) {
	    emit_tree(C, branch);
	}
    }

    emit_end_state(C, si, rc, nest, repc);
    return rc;
}

int parse(unsigned char *input) {
    int rc;
    context_t *C;

    C = &Context;
    branch = &Tree;

    in = input;
    emit_start_state_machine(C);
    rc = parse_r(state_machine,SREP,0,0);
    emit_end_state_machine(C);
    return rc;
}
