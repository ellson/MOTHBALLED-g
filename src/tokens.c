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

// fill buffers from input files
static success_t more_in(context_t *C) {
    int size;

    if (C->in == NULL && feof(C->file)) {  // check if already at EOF
        return FAIL;
    }
    if (C->inbuf) {                  // if there is an existing active-inbuf
	if (C->in == &(C->inbuf->end_of_buf)) {  // if it is full
	    if (( --(C->inbuf->refs)) == 0 ) { // dereference active-inbuf
		free_inbuf(C->inbuf); // free if no refs left
	    }
	    C->inbuf = new_inbuf();  // get new
	    assert(C->inbuf);
	    C->inbuf->refs = 1;      // add active-inbuf reference
	    C->in = C->inbuf->buf;   // point to beginning of buffer
	}
    }
    else {                           // no inbuf, implies just starting
	C->inbuf = new_inbuf();      // get new
	assert(C->inbuf);
	C->inbuf->refs = 1;          // add active-inbuf reference
	C->in = C->inbuf->buf;
    }
    size = fread(C->in, 1, &(C->inbuf->end_of_buf) - C->in, C->file); // slurp in data from file stream
    C->in[size] = '\0';              // ensure terminated (we have an extra character in inbuf_t so this is safe)
    C->insi = char2state[*C->in];

    if (size == 0 && feof(C->file)) {  // check for EOF
        C->in = NULL;
        return FAIL;
    }

    stat_inchars += size;
    return SUCCESS;
}

// consume whitespace fagments
static success_t parse_whitespace_fragment(context_t *C) {
    while (C->insi == WS) {       // eat all leading whitespace
        C->insi = char2state[*++(C->in)];
    }
    return SUCCESS;
}

// consume all whitespace up to next token, or EOF
// FIXME - this function is the place to deal with comments
success_t parse_whitespace(context_t *C) {
    success_t rc;

    rc = SUCCESS;
    parse_whitespace_fragment(C); // eat all leading whitespace
    while (C->insi == NLL) {      // end_of_buffer, or EOF, during whitespace
	if ((rc = more_in(C) == FAIL)) {
	    break;                // EOF
        }
	parse_whitespace_fragment(C); // eat all remaining leading whitespace
    }
    return rc;
}

// load string fragments
static int parse_string_fragment(context_t *C, elem_t *fraglist) {
    unsigned char *frag;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (C->insi == ABC) {
	    frag = C->in;
    	    len = 1;
  	    while ( (C->insi = char2state[*++(C->in)]) == ABC) {len++;}
            elem = new_frag(ABC,len,frag,C->inbuf);
	    slen += len;
        }
        else if (C->insi == AST) {
            // FIXME - flag a pattern;
	    frag = C->in;
    	    len = 1;
  	    while ( (C->insi = char2state[*++(C->in)]) == AST) {}
            elem = new_frag(AST,len,frag,C->inbuf);
	    slen += len;
        }
        else {
	    break;
        }
        append_list(fraglist, elem);
        emit_frag(C,len,frag);
        stat_fragcount++;
    }
    return slen;
}

// collect fragments to form a STRING token
// FIXME - this function is the place to deal with quoting and escaping
success_t parse_string(context_t *C, elem_t *fraglist) {
    success_t rc;
    int len, slen;

    slen = parse_string_fragment(C, fraglist); // leading string
    while (C->insi == NLL) {      // end_of_buffer, or EOF, during whitespace
	if ((rc = more_in(C) == FAIL)) {
	    break;                // EOF
        }
	if ((len = parse_string_fragment(C, fraglist)) == 0) {
	   break;
        }
	slen += len;
    }
    if (slen > 0) {
	stat_stringcount++;
        emit_string(C,fraglist);
        rc = SUCCESS;
    }
    else {
        rc = FAIL;
    }
    return rc;
}

// process single character tokens
success_t parse_token(context_t *C) {
    char token;

    token = state_token[C->insi];
    emit_token(C, token);
    C->insi = char2state[*++(C->in)];
    return SUCCESS;
}
