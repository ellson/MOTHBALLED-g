/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "libje_private.h"

/**
 * fill buffers from input files
 *
 * @param C context
 * @return success/fail
 */
static success_t je_more_in(context_t * C)
{
    int size;

    if (C->inbuf) {        // if there is an existing active-inbuf
        if (C->in == &(C->inbuf->end_of_buf)) {    // if it is full
            if ((--(C->inbuf->refs)) == 0) {    // dereference active-inbuf
                free_inbuf(C, C->inbuf);    // free if no refs left (unlikely)
            }
            new_inbuf(C);    // get new
            assert(C->inbuf);
            C->inbuf->refs = 1;    // add active-inbuf reference
            C->in = C->inbuf->buf;    // point to beginning of buffer
        }
    } else {        // no inbuf, implies just starting
        new_inbuf(C);    // get new
        assert(C->inbuf);
        C->inbuf->refs = 1;    // add active-inbuf reference
        C->in = C->inbuf->buf;
    }

    if (C->file) {        // if there is an existing active input file
        if (C->insi == NLL && feof(C->file)) {    //    if it is at EOF
            //   Although the grammar doesn't care, I decided that it would
            //   be more user-friendly to check that we are not in a quote string
            //   whenever EOF occurs.
            if (C->in_quote) {
                emit_error(C, NLL, "EOF in the middle of a quote string");
            }
// FIXME don't close stdin
// FIXME - stall for more more input   (inotify events ?)
            fclose(C->file);    // then close it and indicate no active input file
            C->file = NULL;
            emit_end_file(C);
        }
    }
    if (!C->file) {        // if there is no active input file
        if (*(C->pargc) > 0) {    //   then try to open the next file
            C->filename = C->argv[0];
            (*(C->pargc))--;
            C->argv = &(C->argv[1]);
            if (strcmp(C->filename, "-") == 0) {
                C->file = stdin;
                *(C->pargc) = 0;    // No files after stdin
            } else {
                C->file = fopen(C->filename, "rb");
                if (!C->file) {
                    emit_error(C, ACTIVITY, "fopen fail");
                }
            }
            C->linecount_at_start = C->stat_lfcount ? C->stat_lfcount : C->stat_crcount;
            C->stat_filecount++;
            emit_start_file(C);
        } else {
            return FAIL;    // no more input available
        }
        assert(C->file);
    }
    // slurp in data from file stream
    size = fread(C->in, 1, &(C->inbuf->end_of_buf) - C->in, C->file);
    C->in[size] = '\0';    // ensure terminated (we have an extras
    //    character in inbuf_t for this )
    C->insi = char2state[*C->in];

    C->stat_inchars += size;
    return SUCCESS;
}

/**
 * consume comment fagments
 *
 * @param C context
 */
static void je_parse_comment_fragment(context_t * C)
{
    unsigned char *in, c;

    in = C->in;
    c = *in;
    while (c != '\0' && c != '\n' && c != '\r') {
        c = *++in;
    }
    C->insi = char2state[c];
    C->in = in;
}

/**
 * consume all comment up to next token, or EOF
 *
 * @param C context
 * @return success/fail
 */
static success_t je_parse_comment(context_t * C)
{
    success_t rc;

    rc = SUCCESS;
    je_parse_comment_fragment(C);    // eat comment
    while (C->insi == NLL) {    // end_of_buffer, or EOF, during comment
        if ((rc = je_more_in(C) == FAIL)) {
            break;    // EOF
        }
        je_parse_comment_fragment(C);    // eat comment
    }
    return rc;
}

/**
 * consume whitespace fagments
 *
 * @ C context
 */
static void je_parse_whitespace_fragment(context_t * C)
{
    unsigned char *in, c;
    state_t insi;

    if ((in = C->in)) {
        c = *in;
        insi = C->insi;
        while (insi == WS) {    // eat all leading whitespace
            if (c == '\n') {
                C->stat_lfcount++;
            }
            if (c == '\r') {
                C->stat_crcount++;
            }
            c = *++in;
            insi = char2state[c];
        }
        C->insi = insi;
        C->in = in;
    }
}

/**
 * consume all non-comment whitespace up to next token, or EOF
 *
 * @param C context
 * @return success/fail
 */
static success_t je_parse_non_comment(context_t * C)
{
    success_t rc;

    rc = SUCCESS;
    je_parse_whitespace_fragment(C);    // eat whitespace
    while (C->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((rc = je_more_in(C) == FAIL)) {
            break;    // EOF
        }
        je_parse_whitespace_fragment(C);    // eat all remaining leading whitespace
    }
    return rc;
}

/**
 * consume all whitespace or comments up to next token, or EOF
 *
 * @param C context
 * @return success/fail
 */
success_t je_parse_whitespace(context_t * C)
{
    success_t rc;

    rc = SUCCESS;
    while (1) {
        if ((rc = je_parse_non_comment(C)) == FAIL) {
            break;
        }
        if (C->insi != OCT) {
            break;
        }
        while (C->insi == OCT) {
            if ((rc = je_parse_comment(C)) == FAIL) {
                break;
            }
        }
    }
    return rc;
}

/**
 * load STRING fragments
 *
 * @param C context
 * @param fraglist - list of frags constituting a string
 * @return length of string
 */
static int je_parse_string_fragment(context_t * C, elem_t * fraglist)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    frag_elem_t *frag_elem;

    slen = 0;
    while (1) {
        if (C->in_quote) {
            if (C->in_quote == 2) {    // character after BSL
                C->in_quote = 1;
                frag = C->in;
                C->insi = char2state[*++(C->in)];
                frag_elem = new_frag(C, BSL, 1, frag);
                slen++;
            } else if (C->insi == DQT) {
                C->in_quote = 0;
                C->insi = char2state[*++(C->in)];
                continue;
            } else if (C->insi == BSL) {
                C->in_quote = 2;
                C->insi = char2state[*++(C->in)];
                continue;
            } else if (C->insi == NLL) {
                break;
            } else {
                frag = C->in;
                len = 1;
                while (1) {
                    insi = char2state[*++(C->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                C->insi = insi;
                frag_elem = new_frag(C, DQT, len, frag);
                slen += len;
            }
        } else if (C->insi == ABC) {
            frag = C->in;
            len = 1;
            while ((insi = char2state[*++(C->in)]) == ABC) {
                len++;
            }
            C->insi = insi;
            frag_elem = new_frag(C, ABC, len, frag);
            slen += len;
        } else if (C->insi == AST) {
            C->has_ast = 1;
            frag = C->in;
            while ((C->insi = char2state[*++(C->in)]) == AST) {
            }    // extra '*' ignored
            frag_elem = new_frag(C, AST, 1, frag);
            slen++;
        } else if (C->insi == DQT) {
            C->in_quote = 1;
            C->has_quote = 1;
            C->insi = char2state[*++(C->in)];
            continue;
        } else {
            break;
        }
        append_list(fraglist, (elem_t*)frag_elem);
        emit_frag(C, len, frag);
        C->stat_fragcount++;
    }
    return slen;
}

/**
 * collect fragments to form a STRING token
 *
 * @param C context
 * @param fraglist
 * @return success/fail
 */
 
success_t je_parse_string(context_t * C, elem_t * fraglist)
{
    int len, slen;

    C->has_quote = 0;
    slen = je_parse_string_fragment(C, fraglist);    // leading string
    while (C->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((je_more_in(C) == FAIL)) {
            break;    // EOF
        }
        if ((len = je_parse_string_fragment(C, fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        C->stat_stringcount++;
        if (C->has_quote) {
            fraglist->state = DQT;
        } else {
            fraglist->state = ABC;
        }
        emit_string(C, fraglist);
        return SUCCESS;
    }
    return FAIL;
}

/**
 * load VSTRING fragments
 *
 * FIXME - add support for additonal quoting formats  (HTML-like, ...)
 *
 * @param C context
 * @param fraglist
 * @return length of string
 */
static int je_parse_vstring_fragment(context_t * C, elem_t * fraglist)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    frag_elem_t *frag_elem;

    slen = 0;
    while (1) {
        if (C->in_quote) {
            if (C->in_quote == 2) {    // character after BSL
                C->in_quote = 1;
                frag = C->in;
                C->insi = char2state[*++(C->in)];
                frag_elem = new_frag(C, BSL, 1, frag);
                slen++;
            } else if (C->insi == DQT) {
                C->in_quote = 0;
                C->insi = char2state[*++(C->in)];
                continue;
            } else if (C->insi == BSL) {
                C->in_quote = 2;
                C->insi = char2state[*++(C->in)];
                continue;
            } else if (C->insi == NLL) {
                break;
            } else {
                frag = C->in;
                len = 1;
                while (1) {
                    insi = char2state[*++(C->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                C->insi = insi;
                frag_elem = new_frag(C, DQT, len, frag);
                slen += len;
            }
        // In the unquoted portions of VSTRING we allow '/' '\' ':' '?'
        // in addition to the ABC class
        // this allows URIs as values without quoting
        } else if (C->insi == ABC ||
                   C->insi == FSL ||
                   C->insi == BSL ||
                   C->insi == CLN ||
                   C->insi == QRY) {
            frag = C->in;
            len = 1;
            while ((insi = char2state[*++(C->in)]) == ABC ||
                    insi == FSL ||
                    insi == BSL ||
                    insi == CLN ||
                    insi == QRY) {
                len++;
            }
            C->insi = insi;
            frag_elem = new_frag(C, ABC, len, frag);
            slen += len;

        // but '*' are still special  (maybe used ias wild card in queries)
        } else if (C->insi == AST) {
            C->has_ast = 1;
            frag = C->in;
            while ((C->insi = char2state[*++(C->in)]) == AST) {
            }    // extra '*' ignored
            frag_elem = new_frag(C, AST, 1, frag);
            slen++;
        } else if (C->insi == DQT) {
            C->in_quote = 1;
            C->has_quote = 1;
            C->insi = char2state[*++(C->in)];
            continue;
        } else {
            break;
        }
        append_list(fraglist, (elem_t*)frag_elem);
        emit_frag(C, len, frag);
        C->stat_fragcount++;
    }
    return slen;
}

/**
 * collect fragments to form a VSTRING token
 *
 * @param C context
 * @param fraglist
 * @return success/fail
 */
success_t je_parse_vstring(context_t * C, elem_t * fraglist)
{
    int len, slen;

    C->has_quote = 0;
    slen = je_parse_vstring_fragment(C, fraglist);    // leading string
    while (C->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((je_more_in(C) == FAIL)) {
            break;    // EOF
        }
        if ((len = je_parse_vstring_fragment(C, fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        C->stat_stringcount++;     //FIXME ?
        if (C->has_quote) {
            fraglist->state = DQT;
        } else {
            fraglist->state = ABC;
        }
        emit_string(C, fraglist);  // FIXME ?
        return SUCCESS;
    } 
    return FAIL;
}

/**
 * process single character tokens
 *
 * @param C context
 * @return success/fail
 */
success_t je_parse_token(context_t * C)
{
    char token;

    token = state_token[C->insi];
    emit_token(C, token);
    C->insi = char2state[*++(C->in)];
    return SUCCESS;
}
