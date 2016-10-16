/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <assert.h>

#include "libje_private.h"

// forward declaration
static void emit_act_func(container_CONTEXT_t * CC, state_t verb, state_t subjtype, elem_t *subject, elem_t *disambig, elem_t *attributes);
static void emit_act_list_r(container_CONTEXT_t *CC, elem_t * list);
static void emit_act_print_frags(state_t liststate, elem_t * elem, char *sep);

static void emit_act_print_token(container_CONTEXT_t *CC, char *tok);

void je_emit_act(container_CONTEXT_t * CC, elem_t *list)
{
    elem_t * elem;
    elemtype_t type;
    int cnt;
    state_t liststate;
    state_t verb = 0;
    state_t subjtype = 0;
    elem_t *subject, *attributes = NULL, *disambig = NULL;
    CC->context->sep = '\0';

    assert(list);
    elem = list->first;
    assert(elem); // must always be a subject
    liststate = (state_t) elem->state;
    if (! (elem->first)) { // is the first elem just a tag?
        switch (liststate) {
        case QRY:
        case TLD:
            verb = liststate;
            break;
        default:
            assert(0); // verb must be query (QRY),  or delete (TLD), (or add if no verb)
            break;
        }
        elem = elem->next;
    }
   
    liststate = (state_t) elem->state;
    switch (liststate) {
    case NODE:
    case SIBLING:
        subject = elem;
        subjtype = liststate;
        break;
    case EDGE:
        subject = elem->first;
        disambig = subject->next;
        subjtype = liststate;
        break;
    default:
        assert(0); // SUBJECT must be NODE,SIBLING, or EDGE
        break;
    }
    elem = elem->next;

    if (elem) {
        liststate = (state_t) elem->state;
        switch (liststate) {
        case ATTRIBUTES:
            attributes = elem;
            break;
        default:
            assert(0); // that should be all
            break;
        }
        elem = elem->next;
    }
    assert(elem == NULL);

    emit_act_func(CC, verb, subjtype, subject, disambig, attributes);
}

static void emit_act_func(container_CONTEXT_t * CC, state_t verb, state_t subjtype, elem_t *subject, elem_t *disambig, elem_t *attributes)
{
    CONTEXT_t *C = CC->context;
    switch (verb) {
    case QRY: putc('?', stdout); break;
    case TLD: putc('~', stdout); break;
    }
    switch (subjtype) {
    case NODE:
//P(subject->first);
        emit_act_list_r(CC, subject->first); // skip NODEID
        break;
    case SIBLING:
//P(subject->first->first);
        emit_act_list_r(CC, subject->first->first); // skip NODEREF NODEID
        break;
    case EDGE:
        putc('<', stdout);
P(subject);
//        emit_act_list_r(CC, subject);
        putc('>', stdout);
        break;
    }
    if (disambig) {
        putc('`', stdout);
//P(disambig->first);
        CC->context->sep = '\0';
        emit_act_list_r(CC, disambig->first); // skip DISAMBID
    }
    if (attributes) {
        putc('[', stdout);
        CC->context->sep = '\0';
//P(attributes);
        emit_act_list_r(CC, attributes);
        putc(']', stdout);
    }
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
    elem = list->first;
    type = (elemtype_t) elem->type;
    switch (type) {
    case FRAGELEM:
        emit_act_print_frags(liststate, elem, &(CC->context->sep));
        break;
    case LISTELEM:
        while (elem) {
            if ((state_t)(elem->state) == EQL) {
                putc('=', stdout);
                CC->context->sep = '\0';
            }
            else {
                emit_act_list_r(CC, elem);    // recurse
            }
            elem = elem->next;
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

