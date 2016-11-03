/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <assert.h>

#include "ikea.h"
#include "emit.h"

// forward declaration
static void ikea_list_r(CONTAINER_t * CONTAINER, elem_t * list);
static void ikea_print_frags(ikea_box_t * ikea_box, state_t liststate, elem_t * elem, char *sep);

void emit_ikea(CONTAINER_t * CONTAINER, elem_t *elem)
{
    CONTAINER->GRAPH->sep = 0; // suppress space before (because preceded by BOF or NL)
    // emit in compact g format
    ikea_list_r(CONTAINER, elem);
    ikea_box_append(CONTAINER->ikea_box, "\n", 1); // NL after each act
}

static void ikea_token(CONTAINER_t * CONTAINER, char *tok)
{
    ikea_box_append(CONTAINER->ikea_box, tok, 1);
    CONTAINER->GRAPH->sep = 0;
}

// recursive function
static void ikea_list_r(CONTAINER_t * CONTAINER, elem_t * list)
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
            ikea_token(CONTAINER, "?");
            break;
        case TLD:
            ikea_token(CONTAINER, "~");
            break;
        default:
            break;
        }
        return;
    }
    type = (elemtype_t) elem->type;
    switch (type) {
    case FRAGELEM:
        ikea_print_frags(CONTAINER->ikea_box, liststate, elem, &(CONTAINER->GRAPH->sep));
        break;
    case LISTELEM:
        cnt = 0;
        while (elem) {
            if (cnt++ == 0) {
                switch (liststate) {
                case EDGE:
                    ikea_token(CONTAINER, "<");
                    break;
                case OBJECT_LIST:
                case ENDPOINTSET:
                    ikea_token(CONTAINER, "(");
                    break;
                case ATTRIBUTES:
                    ikea_token(CONTAINER, "[");
                    break;
                case CONTENTS:
                    ikea_token(CONTAINER, "{");
                    break;
                case VALASSIGN:
                    ikea_token(CONTAINER, "=");
                    break;
                case CHILD:
                    ikea_token(CONTAINER, "/");
                    break;
                default:
                    break;
                }
            }
            ikea_list_r(CONTAINER, elem);    // recurse
            elem = elem->u.l.next;
        }
        switch (liststate) {
        case EDGE:
            ikea_token(CONTAINER, ">");
            break;
        case OBJECT_LIST:
        case ENDPOINTSET:
            ikea_token(CONTAINER, ")");
            break;
        case ATTRIBUTES:
            ikea_token(CONTAINER, "]");
            break;
        case CONTENTS:
            ikea_token(CONTAINER, "}");
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
 * @param ikea_box 
 * @param liststate an indicator if the string is to be quoted
 * @param elem the first frag of the fragllist
 * @param sep if not NULL then a character to be printed first
 */
static void ikea_print_frags(ikea_box_t *ikea_box, state_t liststate, elem_t * elem, char *sep)
{
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
        elem = elem->u.f.next;
    }
    if (liststate == DQT) {
	ikea_box_append(ikea_box, "\"", 1);
    }
    *sep = ' ';
}
