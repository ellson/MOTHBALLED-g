/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "inbuf.h"
#include "list.h"
#include "tree.h"
#include "compare.h"

/**
 * return the larger of two ints
 *
 * @param a integer in the range 0 ... 65535
 * @param b integer in the range 0 ... 65535
 * @return the larger of a or b
 */
static uint16_t max(uint16_t a, uint16_t b)
{
    return a > b ? a : b;
}

/**
 * return the height of an item in the tree, or 0 if p is null
 *
 * @param p a tree item, may be NULL
 * @return p's height, or 0
 */
static uint16_t  height(elem_t * p)
{
    return p ? p->height : 0;
}

/**
 * calculate and update the height of p from the max of left & right, +1
 *
 * @param p the elem whose height is to be recalcualted (non NULL)
 */
static void recalc(elem_t * p)
{
    p->height = 1 + max(height(p->u.t.left), height(p->u.t.right));
}

/**
 * rotate p to the right, recalc heights, and return its old left elem
 *
 * @param p the old root before rotating (non NULL)
 * @return the new root after rotating right (non NULL)
 */
static elem_t * rotate_right(elem_t * p)
{
    elem_t * q;

    q = p->u.t.left;
    p->u.t.left = q->u.t.right;
    q->u.t.right = p;
    recalc(p);
    recalc(q);
    return q;
}

/**
 * rotate p to the left, recalc heights, and return its old right elem
 *
 * @param p the old root before rotating (non NULL)
 * @return the new root after rotating left (non NULL)
 */
static elem_t * rotate_left(elem_t * p)
{
    elem_t * q;

    q = p->u.t.right;
    p->u.t.right = q->u.t.left;
    q->u.t.left = p;
    recalc(p);
    recalc(q);
    return q;
}

/**
 * rotate root left or right as needed to rebalce the tree
 *
 * @param p the old root before rebalancing (non NULL)
 * @return the new root after rebalancing (non NULL)
 */
static elem_t * balance(elem_t * p)
{
    recalc(p);
    if (height(p->u.t.left) - height(p->u.t.right) == 2) {
        if (height(p->u.t.left->u.t.right) > height(p->u.t.left->u.t.left)) {
            p->u.t.left = rotate_left(p->u.t.left);
        }
        return rotate_right(p);
    }
    else {
        if (height(p->u.t.right) - height(p->u.t.left) == 2) {
            if (height(p->u.t.right->u.t.left) > height(p->u.t.right->u.t.right)) {
                p->u.t.right = rotate_right(p->u.t.right);
            }
            return rotate_left(p);
        }
    }
    return p;
}

/**
 * search for an element that matches the key
 *
 * @param p the root of the tree
 * @param key the element to be found
 * @return the matching element, or NULL if not found
 */
elem_t * search_item(elem_t * p, elem_t * key)
{
    int comp;

    if (!p) {
        return NULL;
    }
    comp = compare(key, p->u.t.key);
    if (comp) {
        if (comp < 0) {
            return search_item(p->u.t.left, key);
        }
        else {
            return search_item(p->u.t.right, key);
        }
    }
    return p;        
}

/**
 * Insert an elem into the tree in its sorted position, as determined by compare().
 * If the elem matches an existing elem in the tree, then:
 *      - is value is merged,
 *      - any duplicate bits dereferrenced,
 *      - *newkey updated to point to the key now in the tree.
 *
 * @param LIST the context of the tree
 * @param p the root of the tree
 * @param key - elem being inserted.
 * @param merge function
 * @param newkey - if not NULL, provides a location to store a pointer to the key
 *                 just stored in the tree;  (note the return from insert it
 *                 to the root of the tree, which is not necesarily the node
 *                 of the tree holding the newly inserted key.)
 * @return the new root of the tree after inserting and rebalancing
 */
elem_t * insert_item(LIST_t * LIST, elem_t * p, char state, elem_t *key,
       elem_t * (*merge)(LIST_t* LIST, elem_t *key, elem_t *oldkey),
       elem_t ** newkey)
{
    int comp;
    elem_t *new;

    assert(key);

    if (!p) {
        if (newkey) {
            *newkey = key;
        }
        return new_tree(LIST, state, key);
    }
    comp = compare(key, p->u.t.key);
    if (comp) {
        if (comp < 0) {
            p->u.t.left = insert_item(LIST, p->u.t.left,
                state, key, merge, newkey);
        }
        else {
            p->u.t.right = insert_item(LIST, p->u.t.right,
                state, key, merge, newkey);
        }
    }
    else {
        new = (*merge)(LIST, key, p->u.t.key);
        if (newkey) {
            *newkey = new;
        }
    }
    return balance(p);
}

/**
 * recursively find the minimum elem from the left branch
 *  or maybe p is the min
 *
 * @param p the root of the tree
 * @return the minimum elem (the first elem in ASCII sort order)
 */
static elem_t * find_min(elem_t * p)
{
    if (p->u.t.left != NULL) {
        return find_min(p->u.t.left);
    }
    return p;
}

/**
 * remove the min element, and rebalance the tree
 *
 * @param p the root of the tree
 * @return the minimum elem (the first elem in ASCII sort order)
 */
static elem_t * remove_min(elem_t * p)
{
    if (p->u.t.left == NULL) {
        return p->u.t.right;
    }
    p->u.t.left = remove_min(p->u.t.left);
    return balance(p);
}

/**
 * search for key in the tree, and remove if found
 *
 * @param LIST the context of the tree
 * @param p the root of the tree
 * @param key the elem to be removed
 * @return the new root of the tree after removing and rebalancing (NULL if empty)
 */
elem_t * remove_item(LIST_t * LIST, elem_t * p, elem_t * key)
{
    if (!p) {
        return NULL;
    }
    int comp = compare(key, p->u.t.key);
    if (comp) {
        if (comp < 0) {
            p->u.t.left = remove_item(LIST, p->u.t.left, key);
        }
        else {
            p->u.t.right = remove_item(LIST, p->u.t.right, key);
        }
    }
    else {
        elem_t *l = p->u.t.left;
        elem_t *r = p->u.t.right;
        
        free_tree_item(LIST, p);
    
        if (r == NULL) {
            return l;  // may be NULL
        }
    
        elem_t * m = find_min(r);
        m->u.t.right = remove_min(r);        
        m->u.t.left = l;
    
        return balance(m);
    }
    return balance(p);
}
