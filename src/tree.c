#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "inbuf.h"
#include "list.h"
#include "grammar.h"
#include "frag.h"
#include "compare.h"

static uint16_t max(uint16_t  a, uint16_t  b)
{
    return a > b ? a : b;
}

static uint16_t  height(elem_t * p)
{
    return p ? p->height : 0;
}

static void recalc(elem_t * p)
{
    p->height = 1 + max(height(p->u.t.left), height(p->u.t.right));
}

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

elem_t * search(elem_t * p, elem_t * key)
{
    int comp;

    if (!p) {
        return NULL;
    }
    comp = je_compare(key, p->next);
    if (comp) {
        if (comp < 0) {
            return search(p->u.t.left, key);
        }
        else {
            return search(p->u.t.right, key);
        }
    }
    return p;        
}

void list(elem_t * p, char *sep)
{
    if (!p) {
        return;
    }
    list(p->u.t.left, sep);
    print_frags(stdout, 0, p->next, sep);
    list(p->u.t.right, sep);
}

elem_t * insert(LIST_t * LIST, elem_t * p, elem_t * key)
{
    int comp;

    if (!p) {
        return new_tree(LIST, key);
    }
    comp = je_compare(key, p->next);
    if (comp) {
        if (comp < 0) {
            p->u.t.left = insert(LIST, p->u.t.left, key);
        }
        else {
            p->u.t.right = insert(LIST, p->u.t.right, key);
        }
    }
    else {
        // FIXME - need to merge attributes
        // replace existing key with newer one
        free_list(LIST, p->next);
        p->next = key;
        // FIXME - adjust refs?   Maybe use append()
    }
    return balance(p);
}

static elem_t * find_min(elem_t * p)
{
    if (p->u.t.left != NULL) {
        return find_min(p->u.t.left);
    }
    return p;
}

static elem_t * remove_min(elem_t * p)
{
    if (p->u.t.left == NULL) {
        return p->u.t.right;
    }
    p->u.t.left = remove_min(p->u.t.left);
    return balance(p);
}

elem_t * remove_item(LIST_t * LIST, elem_t * p, elem_t * key)
{
    elem_t  *l, *r, *m;
    int comp;

    if (!p) {
        return NULL;
    }
    comp = je_compare(key, p->next);
    if (comp) {
        if (comp < 0) {
            p->u.t.left = remove_item(LIST, p->u.t.left, key);
        }
        else {
            p->u.t.right = remove_item(LIST, p->u.t.right, key);
        }
    }
    else {
        l = p->u.t.left;
        r = p->u.t.right;
        free_list(LIST, p);
    
        if (r == NULL) {
            return l;
        }
    
        m = find_min(r);
        m->u.t.right = remove_min(r);        
        m->u.t.left = l;
    
        return balance(m);
    }
    return balance(p);
}

#if 0
int main(void)
{
    LIST_t LIST;
    elem_t  *n, *root = NULL;
    char c, sep;
    elem_t *key = NULL;

    while (scanf("%c", &c) && c != 'F') {
        sep = '\0';
        switch (c) {
        case 'A':
//          scanf("%d %d", key);
            root = insert(&LIST, root, key);
            break;
        case 'S':
//            scanf("%d", key);
            n = search(root, key);
            if (n) {
                print_frags(stdout, 0, n->next, &sep);
                printf("\n");
            }
            break;
        case 'L':
            list(root, &sep);
            printf("\n");
            break;
        case 'D':
//            scanf("%d", key);
            root = remove_item(&LIST, root, key);
            break;
        }
    }

    free_tree(&LIST, root);

    return 0;
}
#endif
