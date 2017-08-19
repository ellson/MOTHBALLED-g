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
 * load IDENTIFIER ABC fragments
 *
 * @param TOKEN context
 * @param identifier - list of frags constituting a identifier
 * @return length of identifier
 */
static int identifier_fragment_ABC(TOKEN_t * TOKEN, elem_t * identifier)
{
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    state_t insi;
    unsigned char *frag;
    int len = 0;
    elem_t *elem;

    frag = TOKEN->in;
    while (in != end) {
        insi = char2state[*in];
        if (insi != ABC) {
            TOKEN->insi = insi;
            goto done;
        }
        in++;
        len++;
    }
    TOKEN->insi = END;
done:
    TOKEN->in = in;
    elem = new_frag(LIST(), ABC, len, frag);
    append_transfer(identifier, elem);
    TOKEN->stat_infragcount++;
    return len;
}

/**
 * load IDENTIFIER AST fragments
 *
 * @param TOKEN context
 * @param identifier - list of frags constituting a identifier
 * @return length of identifier
 */
static int identifier_fragment_AST(TOKEN_t * TOKEN, elem_t * identifier)
{
    unsigned char *in = TOKEN->in;
    unsigned char *end = TOKEN->end;
    unsigned char *frag;
    elem_t *elem;

    frag = TOKEN->in;
    while (in != end) {
        if (*in != '*') {
            TOKEN->insi = char2state[*in];
            goto done;
        }
        in++;
    }
    TOKEN->insi = END;
done:
    TOKEN->in = in;
    elem = new_frag(LIST(), AST, 1, frag);
    append_transfer(identifier, elem);
    TOKEN->stat_infragcount++;
    return 1; // only interested in first AST
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
    int len, slen=0;

    assert(identifier);
    assert(identifier->type == (char)LISTELEM);
    assert(identifier->refs > 0);

    do {
        switch (TOKEN->insi) {
            case END:
                if ((token_more_in(TOKEN) == FAIL)) {
                    len = 0; // EOF
                }
                continue;  // continue while
            case ABC:
                len = identifier_fragment_ABC(TOKEN, identifier);
                break;  // break from switch
            case AST:
                TOKEN->elem_has_ast = AST;
                len = identifier_fragment_AST(TOKEN, identifier);
                break;  // break from switch
            default:
                len = 0;
        }
        slen += len;
    } while (len);

    if (slen > 0) {
        // maybe replace fraglist with a shortstr elem
        token_pack_string(TOKEN, slen, identifier);
        return SUCCESS;
    }
    return FAIL;
}
