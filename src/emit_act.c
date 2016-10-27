/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "emit_act.h"

// forward declaration
static void emit_act_func(CONTENT_t * CONTENT, state_t verb, state_t subjtype, elem_t *subject, elem_t *disambig, elem_t *attributes);
static void emit_act_list_r(CONTENT_t * CONTENT, elem_t * list);
static void emit_act_print_frags(state_t liststate, elem_t * elem, char *sep);
static void emit_act_print_shortstr(elem_t * elem, char *sep);

void je_emit_act(CONTENT_t * CONTENT, elem_t *list)
{
    elem_t * elem;
    state_t liststate;
    state_t verb = 0;
    state_t subjtype = 0;
    elem_t *subject, *attributes = NULL, *disambig = NULL;

    assert(list);
    elem = list->u.l.first;
    assert(elem); // must always be a subject
    liststate = (state_t) elem->state;
    if (! (elem->u.l.first)) { // is the first elem just a tag?
        switch (liststate) {
        case QRY:
        case TLD:
            verb = liststate; // override default: 0 = add
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
        subjtype = liststate; // NODE or SIBLING
        break;
    case EDGE:
        subject = elem->u.l.first;  // ENDPOINTS (legs)
        disambig = subject->next; // DISAMBIG (may be NULL)
        subjtype = liststate; // EDGE
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

    emit_act_func(CONTENT, verb, subjtype, subject, disambig, attributes);
}

#define DOTLANG 1
static void emit_act_func(CONTENT_t * CONTENT, state_t verb, state_t subjtype, elem_t *subject, elem_t *disambig, elem_t *attributes)
{
    CONTEXT_t *C = CONTENT->C;
    C->sep = '\0';

    switch (verb) {
    case ACTIVITY: break;               // add
    case TLD: // del
#ifndef DOTLANG
        putc('~', stdout);
#endif
        break;
    case QRY: // qry
#ifndef DOTLANG
        putc('?', stdout);
#endif
        break;
    default:
        assert(0);
        break;
    }
    switch (subjtype) {
    case NODE:
        emit_act_list_r(CONTENT, subject->u.l.first); // skip NODEID
        break;
    case SIBLING:
        emit_act_list_r(CONTENT, subject->u.l.first->u.l.first); // skip NODEREF NODEID
        break;
    case EDGE:
#ifndef DOTLANG
        putc('<', stdout);
        emit_act_list_r(CONTENT, subject);
        putc('>', stdout);
#else
        emit_act_list_r(CONTENT, subject->u.l.first);
        putc('-', stdout);
        putc('-', stdout);
        C->sep = '\0';
        emit_act_list_r(CONTENT, subject->u.l.first->next);
#endif
        break;
    default:
        assert(0); // that should be all
        break;
    }
    if (disambig) {
#ifndef DOTLANG
        putc('`', stdout);
        C->sep = '\0';
        emit_act_list_r(CONTENT, disambig->u.l.first); // skip DISAMBID
#endif
    }
    if (attributes) {
        putc('[', stdout);
        C->sep = '\0';
        emit_act_list_r(CONTENT, attributes);
        putc(']', stdout);
    }
    putc('\n', stdout);
}

// recursive function
static void emit_act_list_r(CONTENT_t * CONTENT, elem_t * list)
{
    elem_t *elem;
    elemtype_t type;
    state_t liststate;

    assert(list);
    elem = list->u.l.first;
    type = (elemtype_t) elem->type;
    switch (type) {
    case FRAGELEM:
        emit_act_print_frags(liststate, elem, &(CONTENT->C->sep));
        break;
    case SHORTSTRELEM:
        emit_act_print_shortstr(elem, &(CONTENT->C->sep));
        break;
    case LISTELEM:
        while (elem) {
            if ((state_t)(elem->state) == EQL) {
                putc('=', stdout);
                CONTENT->C->sep = '\0';
            }
            else {
                emit_act_list_r(CONTENT, elem);    // recurse
            }
            elem = elem->next;
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
 * @param liststate an indicator if the string is to be quoted
 * @param elem the first frag of the fragllist
 * @param sep if not NULL then a character to be printed first
 */
static void emit_act_print_frags(state_t liststate, elem_t * elem, char *sep)
{
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
            fwrite(elem->u.f.frag,1,elem->len,stdout);
        }
       elem = elem->next;
    }
    if (liststate == DQT) {
        putc('"', stdout);
    }
    *sep = ' ';
}

static void emit_act_print_shortstr(elem_t *elem, char *sep)
{
    if (*sep) {
        putc(*sep, stdout);
    }
    fwrite(elem->u.s.str, 1, elem->len, stdout);
    *sep = ' ';
}

