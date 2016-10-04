/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>


#include "libje_private.h"


/**
 * report an error during parsing with context info.
 *
 * @param IN context
 * @param si parser state
 * @param si error message
 */
void je_parse_error(input_t * IN, state_t si, char *message)
{
    unsigned char *p, c;

    fprintf(stderr, "Error: %s ", message);
    print_len_frag(stderr, NAMEP(si));
    fprintf(stderr, "\n      in \"%s\" line: %ld just before: \"",
        IN->filename, (IN->stat_lfcount ?  IN->stat_lfcount : IN->stat_crcount) - IN->linecount_at_start + 1);
    p = IN->in;
    while ((c = *p++)) {
        if (c == '\n' || c == '\r') {
            break;
        }
        putc(c, stderr);
    }
    fprintf(stderr, "\"\n");
    exit(EXIT_FAILURE);
}

/**
 * fill buffers from input files
 *
 * @param C context
 * @return success/fail
 */
static success_t je_more_in(context_t * C)
{
    input_t *IN = &(C->IN);
    INBUFS_t *INBUFS = &(C->LISTS.INBUFS);
    int size;

    if (INBUFS->inbuf) {        // if there is an existing active-inbuf
        if (IN->in == &(INBUFS->inbuf->end_of_buf)) {    // if it is full
            if ((--(INBUFS->inbuf->refs)) == 0) {    // dereference active-inbuf
                free_inbuf(INBUFS, INBUFS->inbuf);    // free if no refs left (unlikely)
            }
            INBUFS->inbuf = new_inbuf(INBUFS);    // get new
            assert(INBUFS->inbuf);
            INBUFS->inbuf->refs = 1;    // add active-inbuf reference
            IN->in = INBUFS->inbuf->buf;    // point to beginning of buffer
        }
    } else {        // no inbuf, implies just starting
        INBUFS->inbuf = new_inbuf(INBUFS);    // get new
        assert(INBUFS->inbuf);
        INBUFS->inbuf->refs = 1;    // add active-inbuf reference
        IN->in = INBUFS->inbuf->buf;
    }

    if (IN->file) {        // if there is an existing active input file
        if (IN->insi == NLL && feof(IN->file)) {    //    if it is at EOF
            //   Although the grammar doesn't care, I decided that it would
            //   be more user-friendly to check that we are not in a quote string
            //   whenever EOF occurs.
            if (IN->in_quote) {
                je_parse_error(IN, NLL, "EOF in the middle of a quote string");
            }
// FIXME don't close stdin
// FIXME - stall for more more input   (inotify events ?)
            fclose(IN->file);    // then close it and indicate no active input file
            IN->file = NULL;
            emit_end_file(C);
        }
    }
    if (!IN->file) {        // if there is no active input file
        if (*(IN->pargc) > 0) {    //   then try to open the next file
            IN->filename = IN->argv[0];
            (*(IN->pargc))--;
            IN->argv = &(IN->argv[1]);
            if (strcmp(IN->filename, "-") == 0) {
                IN->file = stdin;
                *(IN->pargc) = 0;    // No files after stdin
            } else {
                IN->file = fopen(IN->filename, "rb");
                if (!IN->file) {
                    je_parse_error(IN, ACTIVITY, "fopen fail");
                }
            }
            IN->linecount_at_start = IN->stat_lfcount ? IN->stat_lfcount : IN->stat_crcount;
            IN->stat_filecount++;
            emit_start_file(C);
        } else {
            return FAIL;    // no more input available
        }
        assert(IN->file);
    }
    // slurp in data from file stream
    size = fread(IN->in, 1, &(INBUFS->inbuf->end_of_buf) - IN->in, IN->file);
    IN->in[size] = '\0';    // ensure terminated (we have an extras
    //    character in inbuf_t for this )
    IN->insi = char2state[*IN->in];

    IN->stat_inchars += size;
    return SUCCESS;
}

/**
 * consume comment fagments
 *
 * @param C context
 */
static void je_parse_comment_fragment(input_t * IN)
{
    unsigned char *in, c;

    in = IN->in;
    c = *in;
    while (c != '\0' && c != '\n' && c != '\r') {
        c = *++in;
    }
    IN->insi = char2state[c];
    IN->in = in;
}

/**
 * consume all comment up to next token, or EOF
 *
 * @param C context
 * @return success/fail
 */
static success_t je_parse_comment(context_t * C)
{
    input_t *IN = &(C->IN);
    success_t rc;

    rc = SUCCESS;
    je_parse_comment_fragment(IN);    // eat comment
    while (IN->insi == NLL) {    // end_of_buffer, or EOF, during comment
        if ((rc = je_more_in(C) == FAIL)) {
            break;    // EOF
        }
        je_parse_comment_fragment(IN);    // eat comment
    }
    return rc;
}

/**
 * consume whitespace fagments
 *
 * @ IN context
 */
static void je_parse_whitespace_fragment(input_t * IN)
{
    unsigned char *in, c;
    state_t insi;

    if ((in = IN->in)) {
        c = *in;
        insi = IN->insi;
        while (insi == WS) {    // eat all leading whitespace
            if (c == '\n') {
                IN->stat_lfcount++;
            }
            if (c == '\r') {
                IN->stat_crcount++;
            }
            c = *++in;
            insi = char2state[c];
        }
        IN->insi = insi;
        IN->in = in;
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
    input_t *IN = &(C->IN);
    success_t rc;

    rc = SUCCESS;
    je_parse_whitespace_fragment(IN);    // eat whitespace
    while (IN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((rc = je_more_in(C) == FAIL)) {
            break;    // EOF
        }
        je_parse_whitespace_fragment(IN);    // eat all remaining leading whitespace
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
    input_t *IN = &(C->IN);
    success_t rc;

    rc = SUCCESS;
    while (1) {
        if ((rc = je_parse_non_comment(C)) == FAIL) {
            break;
        }
        if (IN->insi != OCT) {
            break;
        }
        while (IN->insi == OCT) {
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
    input_t *IN = &(C->IN);
    LISTS_t *LISTS = &(C->LISTS);
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (IN->in_quote) {
            if (IN->in_quote == 2) {    // character after BSL
                IN->in_quote = 1;
                frag = IN->in;
                IN->insi = char2state[*++(IN->in)];
                elem = new_frag(LISTS, BSL, 1, frag);
                slen++;
            } else if (IN->insi == DQT) {
                IN->in_quote = 0;
                IN->insi = char2state[*++(IN->in)];
                continue;
            } else if (IN->insi == BSL) {
                IN->in_quote = 2;
                IN->insi = char2state[*++(IN->in)];
                continue;
            } else if (IN->insi == NLL) {
                break;
            } else {
                frag = IN->in;
                len = 1;
                while (1) {
                    insi = char2state[*++(IN->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                IN->insi = insi;
                elem = new_frag(LISTS, DQT, len, frag);
                slen += len;
            }
        } else if (IN->insi == ABC) {
            frag = IN->in;
            len = 1;
            while ((insi = char2state[*++(IN->in)]) == ABC) {
                len++;
            }
            IN->insi = insi;
            elem = new_frag(LISTS, ABC, len, frag);
            slen += len;
        } else if (IN->insi == AST) {
            IN->has_ast = 1;
            frag = IN->in;
            while ((IN->insi = char2state[*++(IN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LISTS, AST, 1, frag);
            slen++;
        } else if (IN->insi == DQT) {
            IN->in_quote = 1;
            IN->has_quote = 1;
            IN->insi = char2state[*++(IN->in)];
            continue;
        } else {
            break;
        }
        append_list(fraglist, elem);
        emit_frag(C, len, frag);
        IN->stat_fragcount++;
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
    input_t *IN = &(C->IN);
    int len, slen;

    IN->has_quote = 0;
    slen = je_parse_string_fragment(C, fraglist);    // leading string
    while (IN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((je_more_in(C) == FAIL)) {
            break;    // EOF
        }
        if ((len = je_parse_string_fragment(C, fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        IN->stat_stringcount++;
        if (IN->has_quote) {
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
    input_t *IN = &(C->IN);
    LISTS_t *LISTS = &(C->LISTS);
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (IN->in_quote) {
            if (IN->in_quote == 2) {    // character after BSL
                IN->in_quote = 1;
                frag = IN->in;
                IN->insi = char2state[*++(IN->in)];
                elem = new_frag(LISTS, BSL, 1, frag);
                slen++;
            } else if (IN->insi == DQT) {
                IN->in_quote = 0;
                IN->insi = char2state[*++(IN->in)];
                continue;
            } else if (IN->insi == BSL) {
                IN->in_quote = 2;
                IN->insi = char2state[*++(IN->in)];
                continue;
            } else if (IN->insi == NLL) {
                break;
            } else {
                frag = IN->in;
                len = 1;
                while (1) {
                    insi = char2state[*++(IN->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                IN->insi = insi;
                elem = new_frag(LISTS, DQT, len, frag);
                slen += len;
            }
        // In the unquoted portions of VSTRING we allow '/' '\' ':' '?'
        // in addition to the ABC class
        // this allows URIs as values without quoting
        } else if (IN->insi == ABC ||
                   IN->insi == FSL ||
                   IN->insi == BSL ||
                   IN->insi == CLN ||
                   IN->insi == QRY) {
            frag = IN->in;
            len = 1;
            while ((insi = char2state[*++(IN->in)]) == ABC ||
                    insi == FSL ||
                    insi == BSL ||
                    insi == CLN ||
                    insi == QRY) {
                len++;
            }
            IN->insi = insi;
            elem = new_frag(LISTS, ABC, len, frag);
            slen += len;

        // but '*' are still special  (maybe used ias wild card in queries)
        } else if (IN->insi == AST) {
            IN->has_ast = 1;
            frag = IN->in;
            while ((IN->insi = char2state[*++(IN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LISTS, AST, 1, frag);
            slen++;
        } else if (IN->insi == DQT) {
            IN->in_quote = 1;
            IN->has_quote = 1;
            IN->insi = char2state[*++(IN->in)];
            continue;
        } else {
            break;
        }
        append_list(fraglist, elem);
        emit_frag(C, len, frag);
        IN->stat_fragcount++;
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
    input_t *IN = &(C->IN);
    int len, slen;

    IN->has_quote = 0;
    slen = je_parse_vstring_fragment(C, fraglist);    // leading string
    while (IN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((je_more_in(C) == FAIL)) {
            break;    // EOF
        }
        if ((len = je_parse_vstring_fragment(C, fraglist)) == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        IN->stat_stringcount++;     //FIXME ?
        if (IN->has_quote) {
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
 * @param IN context
 * @return success/fail
 */
success_t je_parse_token(context_t * C)
{
    input_t *IN = &(C->IN);
    char token;

    token = state_token[IN->insi];
    emit_token(C, token);
    IN->insi = char2state[*++(IN->in)];
    return SUCCESS;
}
