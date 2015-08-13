#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "stats.h"
#include "context.h"
#include "emit.h"
#include "token.h"

// fill buffers from input files
static success_t more_in(context_t *C) {
    int size;

    if (C->inbuf) {                   // if there is an existing active-inbuf
	if (C->in == &(C->inbuf->end_of_buf)) {  // if it is full
	    if (( --(C->inbuf->refs)) == 0 ) { // dereference active-inbuf
		free_inbuf(C->inbuf); // free if no refs left
	    }
	    C->inbuf = new_inbuf();   // get new
	    assert(C->inbuf);
	    C->inbuf->refs = 1;       // add active-inbuf reference
	    C->in = C->inbuf->buf;    // point to beginning of buffer
	}
    }
    else {                            // no inbuf, implies just starting
	C->inbuf = new_inbuf();       // get new
	assert(C->inbuf);
	C->inbuf->refs = 1;           // add active-inbuf reference
	C->in = C->inbuf->buf;
    }

    if (C->file) {                    // if there is an existing active input file
        if (C->insi == NLL && feof(C->file)) {  //    if it is at EOF
// FIXME don't close stdin
// FIXME - stall more more input 
	    fclose(C->file);          // then close it and indicate no active input file
	    C->file = NULL;
            emit_end_file(C);
	}
    }
    if (! C->file) {                  // if there is no active input file
        if (*(C->pargc)) {            //   then try to open the next file
	    C->filename = C->argv[0];
	    (*(C->pargc))--;
	    C->argv = &(C->argv[1]);
	    if (strcmp(C->filename,"-") == 0) {
                C->file = stdin;
		*(C->pargc) = 0;      // No files after stdin
            }
            else {
                C->file = fopen(C->filename,"rb");
                if (!C->file) {
                    emit_error(C, ACTIVITY, "fopen fail");
                }
            }
	    C->linecount_at_start = stat_lfcount?stat_lfcount:stat_crcount;
	    stat_filecount++;
            emit_start_file(C);
	}
        else {
	    return FAIL;              // no more input available
        }
        assert(C->file);
    }
                                      // slurp in data from file stream
    size = fread(C->in, 1, &(C->inbuf->end_of_buf) - C->in, C->file);
    C->in[size] = '\0';               // ensure terminated (we have an extras
                                      //    character in inbuf_t for this )
    C->insi = char2state[*C->in];

    stat_inchars += size;
    return SUCCESS;
}

// consume comment fagmentxs
static void parse_comment_fragment(context_t *C) {
    unsigned char *in, c;

    in = C->in;
    c = *in;
    while (c != '\0' && c != '\n' && c != '\r') {
        c = *++in;
    }
    C->insi = char2state[c];
    C->in = in;
}

// consume all comment up to next token, or EOF
static success_t parse_comment(context_t *C) {
    success_t rc;

    rc = SUCCESS;
    parse_comment_fragment(C);    // eat comment
    while (C->insi == NLL) {      // end_of_buffer, or EOF, during comment
	if ((rc = more_in(C) == FAIL)) {
            break;                // EOF
        }
        parse_comment_fragment(C); // eat comment
    }
    return rc;
}

// consume whitespace fagments
static void parse_whitespace_fragment(context_t *C) {
    unsigned char *in, c;
    state_t insi;

    if ((in = C->in)) {
        c = *in;
        insi = C->insi;
        while (insi == WS) {       // eat all leading whitespace
            if (c == '\n') {
                stat_lfcount++;
            }
            if (c == '\r') {
                stat_crcount++;
            }
	    c = *++in;
            insi = char2state[c];
        }
        C->insi = insi;
        C->in = in;
    }
}

// consume all non-comment whitespace up to next token, or EOF
static success_t parse_non_comment(context_t *C) {
    success_t rc;

    rc = SUCCESS;
    parse_whitespace_fragment(C); // eat whitespace
    while (C->insi == NLL) {      // end_of_buffer, or EOF, during whitespace
	if ((rc = more_in(C) == FAIL)) {
	    break;                // EOF
        }
	parse_whitespace_fragment(C); // eat all remaining leading whitespace
    }
    return rc;
}

// consume all whitespace or comments up to next token, or EOF
success_t parse_whitespace(context_t *C) {
    success_t rc;

    rc = SUCCESS;
    while (1) {
	if ((rc = parse_non_comment(C)) == FAIL) {
	    break;
        }
        if (C->insi != OCT) {
            break;
        }
        while  (C->insi == OCT) {
	    if ((rc = parse_comment(C)) == FAIL) {
	        break;
            }
        }
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
            C->ast_seen = 1;
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
