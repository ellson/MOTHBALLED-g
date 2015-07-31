#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "grammar.h"
#include "list.h"
#include "emit.h"
#include "parse.h"

static unsigned char unterm, *in, insep, *frag;
static int len, slen;
static char *insp, subj;
static context_t *C;
elem_t *string;

static int parse_r(char *sp, unsigned char prop, int nest, int repc) {
    unsigned char nprop;
    char *np, insi, ftyp, si, ni, savesubj;
    int rc;
    elem_t *elem;

    si = sp - state_machine;

    emit_start_state(C, si, prop, nest, repc);

    if ((prop & SREP) && repc > 0) {
        emit_sep(C);
    }

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
        if (prop & REP) {
            if (insep == WS) { // whitespace not accepted between REP
	        rc = 1;
	        goto done;
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
    if (si == STRING) { // strinds 
	string = newlist(STRING);
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
            elem = newfrag(ftyp,frag,len,0);
            append_list(string, elem);
	    slen += len;
	}
        insp = state_machine + insi;
	if (slen > 0) {
            emit_string(C,string,slen);
	    free_list(string);
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
	            if (nprop & (REP|SREP)) {
	                while ( parse_r(np,nprop,nest,repc++) == 0) { }
	            }
	        }
                rc = 0; // optional can't fail
	    }
	    else { // else not OPTional
	        if (( rc = parse_r(np,nprop,nest,repc++)) != 0) {
		    break; 
                }
                // rc is the rc of the first term, which at this point is success
	        if (nprop & (REP|SREP)) {
	            while ( parse_r(np,nprop,nest,repc++) == 0) { }
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
    emit_end_state(C, si, rc, nest, repc);
    return rc;
}

static context_t context;

int parse(unsigned char *input) {
    int rc;
    context_t *C;

    C = &context;

    in = input;
    emit_start_state_machine(C);
    rc = parse_r(state_machine,SREP,0,0);
    emit_end_state_machine(C);
    return rc;
}
