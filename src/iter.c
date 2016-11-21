/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "types.h"
#include "inbuf.h"
#include "list.h"
#include "grammar.h"
#include "iter.h"

/**
 * complete the iter state update or the new elem
 *
 * - traversals up, down, or accross lists may contribute
 *   an entra character to be printed (or considered in comparisons or matched)
 *   that depends on the state_t e.g. The '<' '>' that surround edges
 *   and a '\0' to indicate that there is no need for anything between '>' and '<'
 *
 * @param iter - a struct containg the current state of the iterator
 * @param this - the elem for this latest step
 */
static void stepiter(iter_t *iter, elem_t *this)
{
    switch ((elemtype_t)this->type) {
    case FRAGELEM:
        iter->nextstack[iter->sp] = this->u.f.next;
        iter->cp = this->u.f.frag;
        iter->len = this->len;
        break;
    case SHORTSTRELEM:
        iter->nextstack[iter->sp] = NULL;
        iter->cp = this->u.s.str;
        iter->len = this->len;
        break;
    case LISTELEM:
        if ((elemtype_t)this->u.l.first->type == FRAGELEM) {
            // align with SHORTSTRELEM
            this = this->u.l.first;
            iter->nextstack[iter->sp] = this->u.f.next;
            iter->cp = this->u.f.frag;
            iter->len = this->len;
        } else {
            assert(iter->sp < MAXNEST);
            switch ((state_t)this->state) {
            case EDGE:        iter->pps[(iter->sp)] = "<>\0"   ; break;
            case ATTR:        iter->pps[(iter->sp)] = "[] "    ; break;
            case SET:
            case ENDPOINTSET: iter->pps[(iter->sp)] = "()\0"   ; break;
            default:          iter->pps[(iter->sp)] = "\0\0 "  ; break;
            }
            iter->cp = (unsigned char*)iter->pps[(iter->sp)]+0;
            iter->len = 1;
            iter->nextstack[(iter->sp)++] = this->u.l.next;
            iter->nextstack[(iter->sp)] = this->u.l.first;
        }
        break;
    default:
        assert(0);
        break;
    }
}

/**
 * skip the iterator to the end of a chain of elems
 *    and to the next elem in traversal order
 *
 * @param iter - a struct containg the current state of the iterator
 */
void skipiter(iter_t *iter)
{
    elem_t *this;

    if (iter->sp) {
        this = iter->nextstack[--(iter->sp)];
        if (this) {
            switch ((state_t)this->state) {
            case CONTENTS: 
            case ATTRIBUTES:  iter->pps[(iter->sp)] = "\0\0\0" ; break;
            case VALUE:       iter->pps[(iter->sp)] = "\0\0="  ; break;
            default: break;
            }
            iter->cp = (unsigned char*)iter->pps[(iter->sp)]+2;
            iter->len = 1;
            iter->nextstack[(iter->sp)++] = this->u.l.next;
            iter->nextstack[(iter->sp)] = this->u.l.first;
        } else {
            iter->cp = (unsigned char*)iter->pps[(iter->sp)]+1;
            iter->len = 1;
        }
    } else {
        iter->cp = NULL;
        iter->len = 0;
    }
}

/**
 * initialize an iterator for traversing a parser() output list.
 *
 * @param iter - a struct containg the current state of the iterator
 * @param elem - the root elem of the list to be iterated
 */
void inititer(iter_t *iter, elem_t *elem)
{
    assert(iter);
    assert(elem);
    assert((elemtype_t)elem->type == LISTELEM
        || (elemtype_t)elem->type == SHORTSTRELEM);
    stepiter(iter, elem);
}

/**
 * move the iterator to the next elem in traversal order
 *
 * @param iter - a struct containg the current state of the iterator
 */
void nextiter(iter_t *iter)
{
    elem_t *this = iter->nextstack[iter->sp];

    if (this) {
        stepiter(iter, this);
    } else {
        skipiter(iter);
    }
}
