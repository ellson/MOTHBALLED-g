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
 * - Traversals up, down, or accross lists may contribute
 *   an extra character to be printed, or considered in comparisons or matched.
 *   The extra character depends on the state_t e.g. The '<' '>' that surround edges
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
        if (this->u.l.first && (elemtype_t)this->u.l.first->type == FRAGELEM) {
            // align with SHORTSTRELEM
            this = this->u.l.first;
            iter->nextstack[iter->sp] = this->u.f.next;
            iter->cp = this->u.f.frag;
            iter->len = this->len;
        } else {
            assert(iter->sp < MAXNEST);
            switch ((state_t)this->state) {
                case EDGE:        iter->pop_space_push[(iter->sp)] = ">><"   ; break;
                case MUM:         iter->pop_space_push[(iter->sp)] = "\0\0^"   ; break;
                case SET:
                case ENDPOINTSET: iter->pop_space_push[(iter->sp)] = "))("   ; break;
                default:          iter->pop_space_push[(iter->sp)] = "\0 \0"  ; break;
            }
            iter->cp = (unsigned char*)iter->pop_space_push[(iter->sp)]+2;
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
    char save_pop;

    if (iter->sp) {
        this = iter->nextstack[--(iter->sp)];
        if (this) {
            switch ((state_t)this->state) {
                // elems that follow elems of a diferent state_t (non-homogenous lists)
                // need to over-ride the pop_space_push of the preceeding elem
                case ATTRIBUTES:  iter->pop_space_push[(iter->sp)] = "]\0["   ; break;
                case VALUE:       iter->pop_space_push[(iter->sp)] = "\0\0="  ; break;
                case SIS:         iter->pop_space_push[(iter->sp)] = "\0\0\0"   ; break;
                case KID:         iter->pop_space_push[(iter->sp)] = "\0/\0"   ; break;
                default: break;
            }
            // emit space-push (2 chars)
            iter->cp = (unsigned char*)iter->pop_space_push[(iter->sp)]+1;
            iter->len = 2;
            iter->nextstack[(iter->sp)++] = this->u.l.next;
            iter->nextstack[(iter->sp)] = this->u.l.first;
        } else {
            iter->cp = (unsigned char*)iter->pop_space_push[(iter->sp)]+0;
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
