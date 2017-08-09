/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "types.h"
#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "grammar.h"
#include "token.h"
#include "print.h"

#define LIST() ((LIST_t*)TOKEN)

/**
 * load unquoted VSTRING fragment(s)
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int vstring_fragment_ABC(TOKEN_t * TOKEN, elem_t *vstring)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (TOKEN->insi == ABC) { // unquoted string fragment
            frag = TOKEN->in;
            len = 1;
            while ((insi = char2vstate[*++(TOKEN->in)]) == ABC) {
                len++;
            }
            TOKEN->insi = insi;
            elem = new_frag(LIST(), ABC, len, frag);
            slen += len;
        // but '*' are still special  (maybe used as wild card in queries)
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = AST;
            TOKEN->elem_has_ast = AST;
            frag = TOKEN->in;
            while ((TOKEN->insi = char2vstate[*++(TOKEN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LIST(), AST, 1, frag);
            slen++;
        } else {
            break;
        }
        append_transfer(vstring, elem);
        TOKEN->stat_infragcount++;
    }
    return slen;
}

/**
 * load DQT quoted VSTRING fragment(s)
 *
 * Quoting format:
 *     "...\"...\\..." 
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int vstring_fragment_DQT(TOKEN_t * TOKEN, elem_t *vstring)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        switch (TOKEN->in_quote) {
            case 0: // leading quote
                if (TOKEN->insi == DQT) {  // leading quote
                    TOKEN->in_quote = 1;
                    frag = TOKEN->in;
                    len = 1;
                    TOKEN->insi = char2state[*++(TOKEN->in)];
                    slen += len;
                    continue;
                } 
                return slen;
            case 1: // inside quote
                if (TOKEN->insi == DQT) {  // end of quote
                    TOKEN->in_quote = 0;
                    TOKEN->insi = char2state[*++(TOKEN->in)];
                    slen++;
                    elem = new_frag(LIST(), DQT, slen, frag);
                    break;
                } else if (TOKEN->insi == BSL) {  // escape next character
                    TOKEN->in_quote = 2;
                    TOKEN->insi = char2state[*++(TOKEN->in)];
                    slen++;
                    continue;
                } else {  // simple string of ABC
                    len = 1;
                    while ((insi = char2state[*++(TOKEN->in)]) == ABC) {
                        len++;
                    }
                    TOKEN->insi = insi;
                    slen += len;
                    continue;
                }
                break;
            case 2: // escaped character
                TOKEN->in_quote = 1;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                slen++;
                continue;
            default:
                FATAL("shouldn't happen");
        }
        append_transfer(vstring, elem);
        TOKEN->stat_infragcount++;
    }
}

/**
 * load VSTRING fragments
 *
 * Quoting formats:
 *     "..."    strings                    " and \ in strings must be escaped with a \
 
 * coming....
 *     <...>    XML, HTML, ...             unquoted < and > must be properly nested
 *     (...)    Lisp, Guile, Scheme, ...   unquoted ( and ) must be properly nested
 *     {...}    JSON, DOT, ...             unquoted { and } must be properly nested
 *     [nnn]... Binary.                    nnn bytes transparently after the ']'
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int vstring_fragment(TOKEN_t * TOKEN, elem_t *vstring)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
#if 0
        if (TOKEN->quote_type != ABC) {
            // FIXME - extra quoting modes
        } else
#endif
        if (TOKEN->in_quote) {
            if (TOKEN->in_quote == 2) {    // character after escape
                TOKEN->in_quote = 1;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                slen++;
                continue;
            } else if (TOKEN->insi == DQT) {  // end of quote
                TOKEN->in_quote = 0;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                slen++;
                elem = new_frag(LIST(), DQT, slen, frag);
            } else if (TOKEN->insi == BSL) {  // escape next character
                TOKEN->in_quote = 2;
                TOKEN->insi = char2state[*++(TOKEN->in)];
                slen++;
                continue;
            } else if (TOKEN->insi == END) {
                break;
            } else {  // TOKEN->in_quote == 1   .. simple string of ABC
                len = 1;
                while ((insi = char2state[*++(TOKEN->in)]) == ABC) {
                    len++;
                }
                TOKEN->insi = insi;
                slen += len;
                continue;
            }
        } else if (TOKEN->insi == DQT) {  // beginning of quoted string
            TOKEN->in_quote = 1;
            frag = TOKEN->in;
            len = 1;
            while ((insi = char2state[*++(TOKEN->in)]) == ABC) {  // and leading simple string
                len++;
            }
            TOKEN->insi = insi;
            slen += len;
            continue;
        } else if (TOKEN->insi == ABC) { // unquoted string fragment
            frag = TOKEN->in;
            len = 1;
            while ((insi = char2vstate[*++(TOKEN->in)]) == ABC) {
                len++;
            }
            TOKEN->insi = insi;
            elem = new_frag(LIST(), ABC, len, frag);
            slen += len;
        // but '*' are still special  (maybe used as wild card in queries)
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = AST;
            TOKEN->elem_has_ast = AST;
            frag = TOKEN->in;
            while ((TOKEN->insi = char2vstate[*++(TOKEN->in)]) == AST) {
            }    // extra '*' ignored
            elem = new_frag(LIST(), AST, 1, frag);
            slen++;
        } else {
            break;
        }
        append_transfer(vstring, elem);
        TOKEN->stat_infragcount++;
    }
    return slen;
}

/**
 * collect fragments to form a VSTRING token
 *
 * @param TOKEN context
 * @param vstring
 * @return success/fail
 */
success_t token_vstring(TOKEN_t * TOKEN, elem_t *vstring)
{
    int slen = 0;

    assert(vstring);
    assert(vstring->type == (char)LISTELEM);
    assert(vstring->refs > 0);
    TOKEN->has_ast = 0;

    TOKEN->insi = char2vstate[*(TOKEN->in)];
    switch (TOKEN->insi) {
        case ABC:
        case AST:
            slen = vstring_fragment_ABC(TOKEN, vstring);
            break;
        case DQT:
            slen = vstring_fragment_DQT(TOKEN, vstring);
            break;
        default:
            token_error(TOKEN, "Malformed VSTRING", TOKEN->insi);
    }
    while (TOKEN->insi == END) {    // end_of_buffer, or EOF, during whitespace FIXME
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        int len = vstring_fragment(TOKEN, vstring);
        if (len == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        token_pack_string(TOKEN, slen, vstring); // may replace string with a shortstr elem
        return SUCCESS;
    }
    return FAIL;
}
