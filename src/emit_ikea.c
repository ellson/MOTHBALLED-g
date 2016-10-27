/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "ikea.h"
#include "context.h"
#include "emit.h"

// forward declaration
static void ikea_list_r(CONTENT_t * CONTENT, elem_t * list);
static void ikea_print_frags(ikea_box_t * ikea_box, state_t liststate, elem_t * elem, char *sep);

void je_emit_ikea(CONTENT_t * CONTENT, elem_t *elem)
{
    CONTENT->PARSE->sep = 0; // suppress space before (because preceded by BOF or NL)
    // emit in compact g format
    ikea_list_r(CONTENT, elem);
    ikea_box_append(CONTENT->ikea_box, "\n", 1); // NL after each act
}

static void token(CONTENT_t * CONTENT, char *tok)
{
    ikea_box_append(CONTENT->ikea_box, tok, 1);
    CONTENT->PARSE->sep = 0;
}

// recursive function
static void ikea_list_r(CONTENT_t * CONTENT, elem_t * list)
{
    elem_t *elem;
    elemtype_t type;
    int cnt;
    state_t liststate;

    assert(list);
    liststate = (state_t) list->state;
    if (! (elem = list->u.l.first)) {
        switch (liststate) {
        case QRY:
            token(CONTENT, "?");
            break;
        case TLD:
            token(CONTENT, "~");
            break;
        default:
            break;
        }
        return;
    }
    type = (elemtype_t) elem->type;
    switch (type) {
    case FRAGELEM:
        ikea_print_frags(CONTENT->ikea_box, liststate, elem, &(CONTENT->PARSE->sep));
        break;
    case LISTELEM:
        cnt = 0;
        while (elem) {
            if (cnt++ == 0) {
                switch (liststate) {
                case EDGE:
                    token(CONTENT, "<");
                    break;
                case OBJECT_LIST:
                case ENDPOINTSET:
                    token(CONTENT, "(");
                    break;
                case ATTRIBUTES:
                    token(CONTENT, "[");
                    break;
                case CONTAINER:
                    token(CONTENT, "{");
                    break;
                case VALASSIGN:
                    token(CONTENT, "=");
                    break;
                case CHILD:
                    token(CONTENT, "/");
                    break;
                default:
                    break;
                }
            }
            ikea_list_r(CONTENT, elem);    // recurse
            elem = elem->next;
        }
        switch (liststate) {
        case EDGE:
            token(CONTENT, ">");
            break;
        case OBJECT_LIST:
        case ENDPOINTSET:
            token(CONTENT, ")");
            break;
        case ATTRIBUTES:
            token(CONTENT, "]");
            break;
        case CONTAINER:
            token(CONTENT, "}");
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
	    ikea_box_append(ikea_box, (char*)(elem->u.f.frag), elem->len);
        }
       elem = elem->next;
    }
    if (liststate == DQT) {
	ikea_box_append(ikea_box, "\"", 1);
    }
    *sep = ' ';
}

