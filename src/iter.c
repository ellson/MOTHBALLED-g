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
 * complete the iter state update for the new elem
 *
 * - Traversals up, down, or accross lists may contribute
 *   an extra character to be printed, or considered in comparisons or matches.
 *   The extra character depends on the state_t e.g. The '<' '>' that surround edges.
 *   A '\0' is used when there is no need for a character to be printed, but that
 *   there is still a separation of strings when comparing.
 *   e.g.  "abcdef\0ghi"  does not match  "abc\0defghi"
 *
 * @param iter - a struct containg the current state of the iterator
 * @param this - the elem for this latest step
 */
static void stepiter(iter_t *iter, elem_t *this)
{
    switch ((elemtype_t)this->type) {
    case FRAGELEM:
        iter->lnxstack[iter->lsp].lnx = this->u.f.next;
        iter->cp = this->u.f.frag;
        iter->len = this->len;
        break;
    case SHORTSTRELEM:
        iter->lnxstack[iter->lsp].lnx = NULL;
        iter->cp = this->u.s.str;
        iter->len = this->len;
        break;
    case LISTELEM:
        if (this->u.l.first && (elemtype_t)this->u.l.first->type == FRAGELEM) {
            // align with SHORTSTRELEM
            this = this->u.l.first;
            iter->lnxstack[iter->lsp].lnx = this->u.f.next;
            iter->cp = this->u.f.frag;
            iter->len = this->len;
        } else {
            assert(iter->lsp < MAXNEST);
            switch ((state_t)this->state) {
                case ACT:         iter->lnxstack[iter->lsp].psp = "\0\n\0"; break;
                case EDGE:        iter->lnxstack[iter->lsp].psp = ">><"   ; break;
                case MUM:         iter->lnxstack[iter->lsp].psp = "\0\0^" ; break;
                case SET:
                case ENDPOINTSET: iter->lnxstack[iter->lsp].psp = ") ("   ; break;
                default:          iter->lnxstack[iter->lsp].psp = "\0 \0" ; break;
            }
            iter->cp = (unsigned char*)iter->lnxstack[iter->lsp].psp+2;
            iter->len = 1;
            iter->lnxstack[iter->lsp++].lnx = this->u.l.next;
            iter->lnxstack[iter->lsp].lnx = this->u.l.first;
        }
        break;
    case TREEELEM:
        if (iter->tsp == 0) {
            iter->tsp++;
            iter->tnxstack[iter->tsp].tnx = this;
            iter->tnxstack[iter->tsp].dir = 0;
            iter->lnxstack[iter->lsp].psp = "\0 \0";
            iter->lsp++;
        }
        iter->lsp--;
        iter->lnxstack[iter->lsp++].lnx = iter->tnxstack[iter->tsp].tnx;
printf("\nthis %p tsp = %d tnode %p dir %d\n", this, iter->tsp, iter->tnxstack[iter->tsp].tnx, iter->tnxstack[iter->tsp].dir);
        do {
            elem_t *tnode;

            if  (iter->tnxstack[iter->tsp].dir == 0) {
printf("L\n");
                iter->tnxstack[iter->tsp].dir++;
                tnode = iter->tnxstack[iter->tsp].tnx;
                if (tnode->u.t.left) {
                    iter->tsp++;
                    assert (iter->tsp < MAXNEST);
                    iter->tnxstack[iter->tsp].tnx = tnode->u.t.left;
                    iter->tnxstack[iter->tsp].dir = 0;
                    continue;
                }
            }
            if  (iter->tnxstack[iter->tsp].dir == 1) {
printf("N\n");
                iter->tnxstack[iter->tsp].dir++;
                tnode = iter->tnxstack[iter->tsp].tnx;
                iter->lnxstack[iter->lsp].psp = "\0 \0";
                iter->cp = (unsigned char*)iter->lnxstack[iter->lsp].psp+2;
                iter->len = 1;
                iter->lnxstack[iter->lsp++].lnx = tnode;
                this = iter->lnxstack[iter->lsp].lnx = tnode->u.t.key;
                break;
            }
            if (iter->tnxstack[iter->tsp].dir == 2) {
printf("R\n");
                iter->tnxstack[iter->tsp].dir++;
                tnode = iter->tnxstack[iter->tsp].tnx;
                if (tnode->u.t.right) {
                    iter->tsp++;
                    assert (iter->tsp < MAXNEST);
                    iter->tnxstack[iter->tsp].tnx = tnode->u.t.right;
                    iter->tnxstack[iter->tsp].dir = 0;
                    continue;
                }
            }
printf("E\n");
            iter->tsp--;
            assert(iter->lsp);
            if (iter->tsp == 0) {
                iter->lnxstack[iter->lsp].lnx = NULL;
                iter->cp = (unsigned char*)"\0";
                iter->len = 1;
                break;
            }
        } while (1); 
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

    if (iter->lsp) {
        this = iter->lnxstack[--iter->lsp].lnx;
        if (this) {
            switch ((state_t)this->state) {
                // elems that follow elems of a diferent state_t (non-homogenous lists)
                // need to over-ride the pop_space_push of the preceeding elem
                case ATTRIBUTES:  iter->lnxstack[iter->lsp].psp = "]\0["  ; break;
                case DISAMBIG:    iter->lnxstack[iter->lsp].psp = "\0\0`" ; break;
                case VALUE:       iter->lnxstack[iter->lsp].psp = "\0\0=" ; break;
                case SIS:         iter->lnxstack[iter->lsp].psp = "\0 \0" ; break;
                case KID:         iter->lnxstack[iter->lsp].psp = "\0/\0" ; break;
                default: break;
            }
            // emit space-push (2 chars)
            iter->cp = (unsigned char*)iter->lnxstack[iter->lsp].psp+1;
            iter->len = 2;
            iter->lnxstack[iter->lsp++].lnx = this->u.l.next;
            iter->lnxstack[iter->lsp].lnx = this->u.l.first;
        } else {
            iter->cp = (unsigned char*)iter->lnxstack[iter->lsp].psp+0;
            iter->len = 1;
        }
    } else {
        iter->cp = NULL;
        iter->len = 0;
    }
}

/**
 * initialize an iterator for traversing progeny and siblings of elem
 *   ( used in printt() to print key and value )
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
 * initialize an iterator for traversing progeny only of elem
 *   (used in trees where elem and progeny are the key, and siblings are the value)
 *
 * @param iter - a struct containg the current state of the iterator
 * @param elem - the root elem of the list to be iterated
 */
void inititer0(iter_t *iter, elem_t *elem)
{
    assert(iter);
    assert(elem);
    assert((elemtype_t)elem->type == LISTELEM
        || (elemtype_t)elem->type == SHORTSTRELEM);
    stepiter(iter, elem);
    iter->lnxstack[0].lnx = NULL;
}

/**
 * move the iterator to the next elem in traversal order
 *
 * @param iter - a struct containg the current state of the iterator
 */
void nextiter(iter_t *iter)
{
    elem_t *this = iter->lnxstack[iter->lsp].lnx;

    if (this) {
        stepiter(iter, this);
    } else {
        skipiter(iter);
    }
}
