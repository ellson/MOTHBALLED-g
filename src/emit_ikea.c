/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "libje_private.h"

// forward declaration
static void ikea_list_r(container_context_t *CC, elem_t * list);
static void ikea_print_frags(ikea_box_t * ikea_box, state_t liststate, elem_t * elem, char *sep);

void je_emit_ikea(container_context_t * CC, elem_t *elem)
{
    CC->context->sep = 0; // suppress space before (because preceded by BOF or NL)
    // emit in compact g format
    ikea_list_r(CC, elem);
    ikea_box_append(CC->ikea_box, "\n", 1); // NL after each act
}

static void token(container_context_t *CC, char *tok)
{
    ikea_box_append(CC->ikea_box, tok, 1);
    CC->context->sep = 0;
}

// recursive function
static void ikea_list_r(container_context_t *CC, elem_t * list)
{
    elem_t *elem;
    elemtype_t type;
    int cnt;
    state_t liststate;

    assert(list);
    liststate = (state_t) list->state;
    if (! (elem = list->first)) {
        switch (liststate) {
        case QRY:
            token(CC, "?");
            break;
        case TLD:
            token(CC, "~");
            break;
        default:
            break;
        }
        return;
    }
    type = (elemtype_t) elem->type;
    switch (type) {
    case FRAGELEM:
        ikea_print_frags(CC->ikea_box, liststate, elem, &(CC->context->sep));
        break;
    case LISTELEM:
        cnt = 0;
        while (elem) {
            if (cnt++ == 0) {
                switch (liststate) {
                case EDGE:
                    token(CC, "<");
                    break;
                case OBJECT_LIST:
                case ENDPOINTSET:
                    token(CC, "(");
                    break;
                case ATTRIBUTES:
                    token(CC, "[");
                    break;
                case CONTAINER:
                    token(CC, "{");
                    break;
                case VALASSIGN:
                    token(CC, "=");
                    break;
                case CHILD:
                    token(CC, "/");
                    break;
                default:
                    break;
                }
            }
            ikea_list_r(CC, elem);    // recurse
            elem = elem->next;
        }
        switch (liststate) {
        case EDGE:
            token(CC, ">");
            break;
        case OBJECT_LIST:
        case ENDPOINTSET:
            token(CC, ")");
            break;
        case ATTRIBUTES:
            token(CC, "]");
            break;
        case CONTAINER:
            token(CC, "}");
            break;
        default:
            break;
        }
        break;
    default:
        assert(0);  // should not be here
        break;
    }
}

/**
 * Conditionaly print a separator followed by the concatenation of
 * fragments in the list.
 * The composite string is quoted as necessary to comply with g syntax
 * (although not necessarily in the same way as in the original input).
 *
 * @param chan output FILE*
 * @param liststate an indicator if the string is to be quoted
 * @param elem the first frag of the fragllist
 * @param sep if not NULL then a character to be printed first
 */
static void ikea_print_frags(ikea_box_t *ikea_box, state_t liststate, elem_t * elem, char *sep)
{
    unsigned char *frag;
    int len;

    assert(sep);
    if (*sep) {
	ikea_box_append(ikea_box, sep, 1);
    }
    if (liststate == DQT) {
	ikea_box_append(ikea_box, "\"", 1);
    }
    while (elem) {

        assert(elem->type == FRAGELEM);

        if ((state_t) elem->state == BSL) {
	    ikea_box_append(ikea_box, "\\", 1);
        }
        if ((state_t) elem->state == AST) {
            if (liststate == DQT) {
	        ikea_box_append(ikea_box, "\"*\"", 3);
            } else {
	        ikea_box_append(ikea_box, "*", 1);
            }
        }
        else {
	    ikea_box_append(ikea_box,
		((frag_elem_t*)elem)->frag,
		((frag_elem_t*)elem)->len);
        }
       elem = elem->next;
    }
    if (liststate == DQT) {
	ikea_box_append(ikea_box, "\"", 1);
    }
    *sep = ' ';
}

