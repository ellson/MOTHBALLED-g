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
static void emit_act_list_r(container_CONTEXT_t *CC, elem_t * list);
static void emit_act_print_frags(state_t liststate, elem_t * elem, char *sep);

static void emit_act_print_token(container_CONTEXT_t *CC, char *tok);

void je_emit_act(container_CONTEXT_t * CC, elem_t *elem)
{
    CC->context->sep = 0; // suppress space before (because preceded by BOF or NL)
    emit_act_list_r(CC, elem);
    putc('\n', stdout);
}

// recursive function
static void emit_act_list_r(container_CONTEXT_t *CC, elem_t * list)
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
            emit_act_print_token(CC, "?");
            break;
        case TLD:
            emit_act_print_token(CC, "~");
            break;
        default:
            break;
        }
        return;
    }
    type = (elemtype_t) elem->type;
    switch (type) {
    case FRAGELEM:
        emit_act_print_frags(liststate, elem, &(CC->context->sep));
        break;
    case LISTELEM:
        cnt = 0;
        while (elem) {
            if (cnt++ == 0) {
                switch (liststate) {
                case EDGE:
                    emit_act_print_token(CC, "<");
                    break;
                case OBJECT_LIST:
                case ENDPOINTSET:
                    assert(0); // lists should be fully expanded at this point
                    break;
                case ATTRIBUTES:
                    emit_act_print_token(CC, "[");
                    break;
                case CONTAINER:
                    emit_act_print_token(CC, "{");
                    break;
                case VALASSIGN:
                    emit_act_print_token(CC, "=");
                    break;
                case CHILD:
                    emit_act_print_token(CC, "/");
                    break;
                default:
                    break;
                }
            }
            emit_act_list_r(CC, elem);    // recurse
            elem = elem->next;
        }
        switch (liststate) {
        case EDGE:
            emit_act_print_token(CC, ">");
            break;
        case OBJECT_LIST:
        case ENDPOINTSET:
            assert(0); // lists should be fully expanded at this point
            break;
        case ATTRIBUTES:
            emit_act_print_token(CC, "]");
            break;
        case CONTAINER:
            emit_act_print_token(CC, "}");
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

static void emit_act_print_token(container_CONTEXT_t *CC, char *tok)
{
    putc(*tok, stdout);
    CC->context->sep = 0;
}

/**
 * Conditionaly print a separator followed by the concatenation of
 * fragments in the list.
 * The composite string is quoted as necessary to comply with g syntax
 * (although not necessarily in the same way as in the original input).
 *
 * @param liststate an indicator if the string is to be quoted
 * @param elem the first frag of the fragllist
 * @param sep if not NULL then a character to be printed first
 */
static void emit_act_print_frags(state_t liststate, elem_t * elem, char *sep)
{
    unsigned char *frag;
    int len;

    assert(sep);
    if (*sep) {
        putc(*sep, stdout);
    }
    if (liststate == DQT) {
        putc('"', stdout);
    }
    while (elem) {

        assert(elem->type == FRAGELEM);

        if ((state_t) elem->state == BSL) {
            putc('\\', stdout);
        }
        if ((state_t) elem->state == AST) {
            if (liststate == DQT) {
                fwrite("\"*\"",1,3,stdout);
            } else {
                putc('*', stdout);
            }
        }
        else {
            fwrite(((frag_elem_t*)elem)->frag,1,((frag_elem_t*)elem)->len,stdout);
        }
       elem = elem->next;
    }
    if (liststate == DQT) {
        putc('"', stdout);
    }
    *sep = ' ';
}

