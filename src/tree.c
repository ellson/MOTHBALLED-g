#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#if 1
struct node
{
    int key;
    int data;    

    int height;

    struct node* left;
    struct node* right;
};

typedef struct node node;
#endif 

#include "inbuf.h"
#include "grammar.h"
#include "list.h"

static node* new_node(int key, int data)
{
    node* p;

    p = malloc(sizeof(node));
//FIXME - deal will malloc fail
    p -> key    = key;
    p -> data   = data;
    p -> height = 1;
    p -> left   = NULL;
    p -> right  = NULL;

    return p;
}

static int max(int a, int b)
{
    return a > b ? a : b;
}

static int height(node* p)
{
    return p ? p -> height : 0;
}

void recalc(node* p)
{
    p -> height = 1 + max(height(p -> left), height(p -> right));
}

static node* rotate_right(node* p)
{
    node* q;

    q = p -> left;
    p -> left = q -> right;
    q -> right = p;
    recalc(p);
    recalc(q);
    return q;
}

static node* rotate_left(node* p)
{
    node* q;

    q = p -> right;
    p -> right = q -> left;
    q -> left = p;
    recalc(p);
    recalc(q);
    return q;
}

static node* balance(node* p)
{
    recalc(p);
    if ( height(p -> left) - height(p -> right) == 2 ) {
        if ( height(p -> left -> right) > height(p -> left -> left) ) {
            p -> left = rotate_left(p -> left);
        }
        return rotate_right(p);
    }
    else {
        if ( height(p -> right) - height(p -> left) == 2 ) {
            if ( height(p -> right -> left) > height(p -> right -> right) ) {
                p -> right = rotate_right(p -> right);
            }
            return rotate_left(p);
        }
    }
    return p;
}

static node* search(node* p, int key)
{
    if ( !p ) {
        return NULL;
    }
    if ( key < p -> key ) {
        return search(p -> left, key);
    }
    else {
        if ( key > p -> key ) {
            return search(p -> right, key);
        }
    }
    return p;        
}

static void list(node* p)
{
    if ( !p ) {
        return;
    }
    list( p -> left);
    printf("%d %d\n", p -> key, p -> data);
    list( p -> right);
}

static node* insert(node* p, int key, int data)
{
    if ( !p ) {
        return new_node(key, data);
    }
    if ( key < p -> key ) {
        p -> left = insert(p -> left, key, data);
    }
    else {
        if ( key > p -> key ) {
            p -> right = insert(p -> right, key, data);
        }
        else {
            p -> data = data;
        }
    }
    return balance(p);
}

static node* find_min(node* p)
{
    if ( p -> left != NULL ) {
        return find_min(p -> left);
    }
    return p;
}

static node* remove_min(node* p)
{
    if ( p -> left == NULL ) {
        return p -> right;
    }
    p -> left = remove_min(p -> left);
    return balance(p);
}

static node* remove_item(node* p, int key)
{
    node *l, *r, *m;

    if ( !p ) {
        return NULL;
    }
    if ( key < p -> key ) {
        p -> left = remove_item(p -> left, key);
    }
    else {
        if ( key > p -> key ) {
            p -> right = remove_item(p -> right, key);
        }
        else
        {
            l = p -> left;
            r = p -> right;
            free(p);
    
            if ( r == NULL ) {
                return l;
            }
    
            m = find_min(r);
            m -> right = remove_min(r);        
            m -> left = l;
    
            return balance(m);
        }
    }
    return balance(p);
}

static void free_tree(node* p)
{
    if ( !p ) {
        return;
    }
    free_tree(p -> left);
    free_tree(p -> right);
    free(p);
}

int main(void)
{
    node *n, *root = NULL;
    char c;
    int k, d;

    while ( scanf("%c", &c) && c != 'F' ) {
        switch (c) {
        case 'A':
            scanf("%d %d", &k, &d);
            root = insert(root, k, d);
            break;
        case 'S':
            scanf("%d", &k);
            n = search(root, k);
            if ( n ) {
                printf("%d %d\n", n -> key, n -> data);
            }
            break;
        case 'L':
            list(root);
            break;
        case 'D':
            scanf("%d", &k);
            root = remove_item(root, k);
            break;
        }
    }

    free_tree(root);

    return 0;
}
