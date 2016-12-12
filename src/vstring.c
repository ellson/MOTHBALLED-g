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
 * load VSTRING fragments
 *
 * Supports additonal quoting formats:
 *     <...>    XML, HTML, ...             unquoted < and > must be properly nested
 *     (...)    Lisp, Guile, Scheme, ...   unquoted ( and ) must be properly nested
 *     {...}    JSON, DOT, ...             unquoted { and } must be properly nested
 *     [nnn]... Binary.                    nnn bytes transparently after the ']'
 *
 * @param TOKEN context
 * @param vstring
 * @return length of vstring
 */
static int token_vstring_fragment(TOKEN_t * TOKEN, elem_t *vstring)
{
    unsigned char *frag;
    state_t insi;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (TOKEN->quote_type != ABC) {
            // FIXME - extra quoting modes
        } else if (TOKEN->in_quote) {
            if (TOKEN->in_quote == 2) {    // character after BSL
                TOKEN->in_quote = 1;
                frag = TOKEN->in;
                TOKEN->insi = char2vstate[*++(TOKEN->in)];
                elem = new_frag(LIST(), BSL, 1, frag);
                slen++;
            } else if (TOKEN->insi == DQT) {
                TOKEN->in_quote = 0;
                TOKEN->insi = char2vstate[*++(TOKEN->in)];
                continue;
            } else if (TOKEN->insi == BSL) {
                TOKEN->in_quote = 2;
                TOKEN->insi = char2vstate[*++(TOKEN->in)];
                TOKEN->has_bsl = BSL;
                continue;
            } else if (TOKEN->insi == NLL) {
                break;
            } else {
                frag = TOKEN->in;
                len = 1;
                while (1) {
                    insi = char2vstate[*++(TOKEN->in)];
                    if (insi == DQT || insi == BSL || insi == NLL) {
                        break;
                    }
                    len++;
                }
                TOKEN->insi = insi;
                elem = new_frag(LIST(), DQT, len, frag);
                slen += len;
            }
        } else if (TOKEN->insi == DQT) {
            TOKEN->in_quote = 1;
            TOKEN->quote_state = DQT;
            TOKEN->insi = char2vstate[*++(TOKEN->in)];
            continue;
        } else if (TOKEN->insi == ABC) {
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
    assert(vstring);
    assert(vstring->type == (char)LISTELEM);
    assert(vstring->refs > 0);
    TOKEN->has_ast = 0;
    TOKEN->has_bsl = 0;

    TOKEN->quote_state = ABC;
    TOKEN->quote_type = ABC;
    TOKEN->insi = char2vstate[*(TOKEN->in)]; // recheck the first char against expanded set
    if (TOKEN->insi != ABC) {
        if (TOKEN->insi == LPN || TOKEN->insi == LAN || TOKEN->insi == LBE || TOKEN->insi == LBR) {
            // balanced paren quoting or binary quoting modes
            TOKEN->quote_type = TOKEN->insi;
            TOKEN->quote_counter = 0;
        } else if ( ! (TOKEN->insi == DQT || TOKEN->insi == AST || TOKEN->insi == BSL)) {
            token_error(TOKEN, "Malformed VSTRING", TOKEN->insi);
        }
    }
    int slen = token_vstring_fragment(TOKEN, vstring);    // leading string
    while (TOKEN->insi == NLL) {    // end_of_buffer, or EOF, during whitespace
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        int len = token_vstring_fragment(TOKEN, vstring);
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
