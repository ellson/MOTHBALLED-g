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

// I wanted to turn QUOTING_IN_IDENTIFIERS off, to make it more C like, and to
// enforce a cleaner coding style, IMHO.   Pragmatically though, if we want
// to allow translations from other languages, like DOT or JSON, then we
// may neeed to support quoting in identifiers.

#ifndef QUOTING_IN_IDENTIFIERS
#define QUOTING_IN_IDENTIFIERS 0
#endif

/**
 * load IDENTIFIER fragments
 *
 * @param TOKEN context
 * @param identifier - list of frags constituting a identifier
 * @return length of identifier
 */
static int token_identifier_fragment(TOKEN_t * TOKEN, elem_t * identifier)
{
    unsigned char *frag;
    int slen, len;
    elem_t *elem;

    slen = 0;
    while (1) {
        if (TOKEN->insi == ABC) {
            frag = TOKEN->in;
            len = token_n(TOKEN, ABC);
            elem = new_frag(LIST(), ABC, len, frag);
            slen += len;
        } else if (TOKEN->insi == AST) {
            TOKEN->has_ast = AST;
            TOKEN->elem_has_ast = AST;
            frag = TOKEN->in;
            len = token_n(TOKEN, AST);  // extra '*' ignored
            elem = new_frag(LIST(), AST, 1, frag);
            slen++;
        } else {
            break;
        }
        append_transfer(identifier, elem);
        TOKEN->stat_infragcount++;
    }
    return slen;
}

/**
 * collect fragments to form an IDENTIFIER token
 *
 * @param TOKEN context
 * @param identifier
 * @return success/fail
 */

success_t token_identifier(TOKEN_t * TOKEN, elem_t *identifier)
{
    assert(identifier);
    assert(identifier->type == (char)LISTELEM);
    assert(identifier->refs > 0);

    TOKEN->has_ast = 0;
    int slen = token_identifier_fragment(TOKEN, identifier); // leading fragment
    while (TOKEN->insi == END) {    // end_of_buffer, or EOF, during whitespace
        if ((token_more_in(TOKEN) == FAIL)) {
            break;    // EOF
        }
        int len = token_identifier_fragment(TOKEN, identifier);
        if (len == 0) {
            break;
        }
        slen += len;
    }
    if (slen > 0) {
        token_pack_string(TOKEN, slen, identifier); // may replace identifier with a shortstr elem
        return SUCCESS;
    }
    return FAIL;
}
