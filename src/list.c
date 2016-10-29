/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "list.h"

/**
 * Private function to manage the allocation an elem_t
 *
 * elem_t are allocated in blocks and maintained in a free_elem_t list.
 * Freeing an elem_t actually means returning to this list.
 *
 * @param LIST the top-level context in which all lists are managed
 * @return a new intialized elem_t
 */
static elem_t *new_elem_sub(LIST_t * LIST)
{
    elem_t *elem, *next;
    int i;

    if (!LIST->free_elem_list) {    // if no elems in free_elem_list

        LIST->free_elem_list = malloc(LISTALLOCNUM * sizeof(elem_t));
        if (!LIST->free_elem_list)
            fatal_perror("Error - malloc(): ");
        LIST->stat_elemmalloc++;

        next = LIST->free_elem_list;    // link the new elems into free_elem_list
        i = LISTALLOCNUM;
        while (i--) {
            elem = next++;
            elem->next = next;
        }
        elem->next = NULL;    // terminate last elem

    }
    elem = LIST->free_elem_list;    // use first elem from free_elem_list
    LIST->free_elem_list = elem->next; // update list to point to next available

    LIST->stat_elemnow++;        // stats
    if (LIST->stat_elemnow > LIST->stat_elemmax) {
        LIST->stat_elemmax = LIST->stat_elemnow;
    }

    // N.B. elem is uninitialized
    return elem;
}

/**
 * Return a pointer to an elem_t which is an empty list
 * The elem_t is memory managed without caller involvement.
 *
 * @param LIST the top-level context in which all lists are managed
 * @param state a one character value stored with the elem, no internal meaning
 * @return a new intialized elem_t
 */
elem_t *new_list(LIST_t * LIST, char state)
{
    elem_t *elem;

    elem = new_elem_sub(LIST);

    assert(elem);
    // complete elem initialization
    elem->type = LISTELEM;
    elem->next = NULL;      // clear next
    elem->state = state;    // state_machine state that created this frag
    elem->u.l.first = NULL; // new list is empty
    elem->u.l.last = NULL;
    elem->len = 0;
    elem->refs = 1;

    return elem;
}

/**
 * Return a pointer to an elem_t which is a tree node with a key (reference too a list)
 * The elem_t is memory managed without caller involvement.
 *
 * @param LIST the top-level context in which all lists are managed
 * @param key a list containing (as some point) some frags wihich are the key
 * @return a new intialized elem_t
 */
elem_t *new_tree(LIST_t * LIST, elem_t *key)
{
    elem_t *elem;

    assert(key);
    assert(key->type == (char)LISTELEM);
    key->refs++;
    assert(key->refs > 0);

    elem = new_elem_sub(LIST);
    assert(elem);

    // complete elem initialization
    elem->type = TREEELEM;
    elem->next = key; 
    elem->u.t.left = NULL; // new tree is empty so far
    elem->u.t.right = NULL;
    elem->height = 1;
    elem->state = 0; //notused
    elem->len = 0;   //notused
    elem->refs = 0;  //notused

    return elem;
}

void free_tree_item(LIST_t *LIST, elem_t * p)
{
    assert(p->next == (char)LISTELEM);
    p->next->refs--;
    free_list_content(LIST, p->next);

    // return p to the freelist
    p->next = LIST->free_elem_list;
    LIST->free_elem_list = p;
    LIST->stat_elemnow--;    // maintain stats
}

void free_tree(LIST_t *LIST, elem_t * p)
{
    if ( !p ) {
        return;
    }
    assert(p->type = (char)TREEELEM);
    free_tree(LIST, p->u.t.left);
    free_tree(LIST, p->u.t.right);
    free_tree_item(LIST, p);
}

/**
 * Return a pointer to an elem_t which holds a string fragment
 * (start address and length).
 * The elem_t is memory managed without caller involvement.
 *
 * @param LIST the top-level context in which all lists are managed
 * @param state a one character value stored with the elem, no internal meaning
 * @param len fragment length
 * @param frag pointer to first character of contiguous fragment of len chars
 * @return a new intialized elem_t
 */
elem_t *new_frag(LIST_t * LIST, char state, uint16_t len, unsigned char *frag)
{
    INBUF_t * INBUF = &(LIST->INBUF);
    elem_t *elem;

    assert(INBUF->inbuf);
    assert(INBUF->inbuf->refs >= 0);
    assert(frag);
    assert(len > 0);

    elem = new_elem_sub(LIST);

    // complete frag elem initialization
    elem->type = FRAGELEM;  // type
    elem->next = NULL;      // clear next
    elem->state = state;    // state_machine state that created this frag
    elem->u.f.inbuf = INBUF->inbuf; // record inbuf for ref counting
    elem->u.f.frag = frag;  // pointer to begging of frag
    elem->len = len;    // length of frag

    INBUF->inbuf->refs++;   // increment reference count in inbuf.
    return elem;
}

/**
 * Return a pointer to an elem_t which holds a shortstr
 * (suitable for use as a hash name for hubs)
 * The string is stored in the struct.  Maximum length is the first 16 characters.
 * of the referenced str.  The stored string is *not* NUL terminated;
 *
 * @param LIST the top-level context in which all lists are managed
 * @param state a one character value stored with the elem, no internal meaning
 * @param str string to be strored in elem,  first 16 chars max
 * @return a new intialized elem_t
 */
elem_t *new_shortstr(LIST_t * LIST, char state, char * str)
{
    elem_t *elem;
    int i;
    char c;

    elem = new_elem_sub(LIST);

    // complete frag elem initialization
    elem->type = SHORTSTRELEM; // type
    elem->next = NULL;         // clear next
    elem->state = state;       // state_machine state that created this shortstr
    for (i = 0; i < sizeof(((elem_t*)0)->u.s.str) && (c = str[i]) != '\0'; i++) {
        elem->u.s.str[i] = c;
    }
    elem->len = i;
    return elem;
}

/**
 * Return a pointer to an elem_t which holds a hashname
 * (suitable for use as a filename)
 * The element is memory managed without caller involvement.
 * The FILE* in the elem_t is initialized to NULL
 *
 * @param LIST the top-level context in which all lists are managed
 * @param hash a long containing a hash value
 * @param hash_len  
 * @return a new intialized elem_t
 */
elem_t *new_hashname(LIST_t * LIST, unsigned char* hash, size_t hash_len)
{
    elem_t *elem;

    elem = new_elem_sub(LIST);

    // complete frag elem initialization
    elem->type = HASHNAMEELEM;  // type
    elem->next = NULL;          // clear next
    elem->state = 0;            // state_machine state that created this shortstr
    elem->u.h.hashname = hash;  // the hash value  //FIXME do base64 here ?
    elem->u.h.out = NULL;       // open later
    elem->len = hash_len;       // FIXME - is this used?
    return elem;
}

/**
 * Create a new list with the same content, by reference, as the input list.
 *
 * @param LIST the top-level context in which all lists are managed
 * @param list a header to a list to be referenced
 * @return a new intialized elem_t which is now also a header of the referenced list
 */
elem_t *ref_list(LIST_t * LIST, elem_t * list)
{
    elem_t *elem;

    assert(list->type == (char)LISTELEM);

    elem = new_elem_sub(LIST);

    elem->type = LISTELEM;       // type
    elem->next = NULL;           // clear next
    elem->refs = 0;              // no refs to this yet
    elem->state = list->state;
    elem->u.l.first = list->u.l.first; // copy details
    elem->u.l.last = list->u.l.last;
    elem->len = list->len;
    if (list->u.l.first && list->u.l.first->type == LISTELEM) {
        list->u.l.first->refs++;    // increment ref count
    }
    return elem;
}

/**
 *  Append a list elem_t to the end of the list of lists.
 *
 *  The reference count in the appended element is incremented
 *  to account for the new reference from the old tail elem_t
 *
 * @param list the header of the list to be appended
 * @param elem the element to be appended (must be a LISTELEM)
 */
void append_list(elem_t * list, elem_t * elem)
{
    assert(list->type == (char)LISTELEM);

    if (list->u.l.first) {
        list->u.l.last->next = elem;
    } else {
        list->u.l.first = elem;
    }
    list->len++;
    list->u.l.last = elem;
    if (elem->type == (char)LISTELEM) {
        elem->refs++; 
        assert(elem->refs > 0);
    }
}

/**
 *  Remove the next element from a list.
 *
 *  The removed element is freed.
 *
 * @param LIST the top-level context in which all lists are managed
 * @param list header of the list to be shortened
 * @param elem the elem preceeding the elem to be removed (or NULL to remove 1st elem)
 */
void remove_next_from_list(LIST_t * LIST, elem_t * list, elem_t *elem)
{
    elem_t *old;

    assert(list);
    assert(list->type == (char)LISTELEM);
    assert(list->u.l.last);                  // must be at least one elem in the list

    if (! elem) {                            // if removing the first elem
        old = list->u.l.first;
        list->u.l.first = list->u.l.first->next; // skip the elem being removed
    }
    else {
        old = elem->next;
        elem->next = elem->next->next;       // skip the elem being removed
    }
    if (list->u.l.last == old) {             // if removing the last element
        list->u.l.last = elem;               // then elem is the new last (or NULL)
    }
    list->len--;                             // list has one less elem
    free_list_content(LIST, old);            // free the removed elem
}

/**
 * Free the contents of a list, but not the list itself
 *  
 * If it is a list of lists, then the refence count in the first elem_t is
 * decremented and the elements are freed only if the references are  zero.
 *
 * If it is a list of fragments, then the reference count to the fragments'
 * inbufs are decremented and the inbuf freed if there are no more fragments
 * in use.
 *
 * Lists of hashes are not allowed to be freed.
 *
 * @param LIST the top-level context in which all lists are managed
 * @param list a header to the list to be freed.
 */
void free_list_content(LIST_t * LIST, elem_t * list)
{
    assert(list);
    assert(list->type == (char)LISTELEM);

    // free list of elem, but really just put them back
    // on the elem_freelist (declared at the top of this file)`
    free_list(LIST, list->u.l.first);

    // clean up emptied list
    list->u.l.first = NULL;
    list->u.l.last = NULL;
    // Note: ref count of the empty list is not modified.
    // It may be still referenced, even though it is now empty.
}


/**
 * Free list - for each elem in list: free contents, then free the emmpty list,
 * (the elem_t heading the list) but only if its ref count is 0
 *  
 *
 * @param LIST the top-level context in which all lists are managed
 * @param elem - a list header
 */
void free_list(LIST_t * LIST, elem_t * elem)
{
    elem_t *next;

    while (elem) {
        next = elem->next;
        switch ((elemtype_t)(elem->type)) {
        case LISTELEM:
            assert(elem->refs > 0);
            if (--(elem->refs)) {
                return;    // stop at any point with additional refs
            }
            free_list_content(LIST, elem); // recursively free lists that have no references
            break;
        case FRAGELEM:
            assert(elem->u.f.inbuf->refs > 0);
            if (--(elem->u.f.inbuf->refs) == 0) {
                free_inbuf(&(LIST->INBUF), elem->u.f.inbuf);
            }
            break;
        case SHORTSTRELEM:
            // these are self contained singletons,  nothing else to clean up
            next = NULL;
            break;
        case TREEELEM:  // FIXME - we will need this
        case HASHNAMEELEM:
            assert(0);  // should not be here
            break;
        }
        // insert elem at beginning of freelist
        elem->next = LIST->free_elem_list;
        LIST->free_elem_list = elem;
        LIST->stat_elemnow--;    // maintain stats

        elem = next;
    }
}
