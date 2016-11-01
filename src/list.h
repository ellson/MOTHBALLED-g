/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "inbuf.h"

typedef enum {
    LISTELEM = 0,           // must be 0 for static or calloc allocation of list headers
    FRAGELEM = 1,
    SHORTSTRELEM = 2,
    TREEELEM = 3
} elemtype_t;

typedef struct elem_s elem_t;
struct elem_s { 
    union {
        struct {
            elem_t *next;          // next elem in parent's list
            elem_t *first;         // first elem in this list (or NULL)
            elem_t *last;          // last elem in this list (or NULL)
        } l;
        struct {
            elem_t *next;          // next frag in fraglist list
            inbuf_t *inbuf;        // inbuf containing frag - for memory management
            unsigned char *frag;   // pointer to beginning of frag
        } f;
        struct {
            unsigned char str[sizeof(void*)*3]; // short string (12char on 32bit machines)
        } s;
        struct {
            elem_t *key;           // a list containg the key for this node in the tree
            elem_t *left;          // left elem of tree
            elem_t *right;         // left elem of tree
        } t;
    } u;
    uint16_t height;        // (belongs to u.t.) height of elem in tree
    int16_t refs;           // (belongs to u.l.) don't free this list until refs == 0
    uint16_t len;           // (shared by u.l. u.f. u.s.)
    char state;             // state_machine state that generated this elem
    char type;              // just this *has* to be outside of union
                            //  but to move the rest inside would increase the
                            //  size of the struct
};

typedef struct {
    INBUF_t INBUF;          // Header for inbuf management   
    elem_t *free_elem_list; // linked list of unused list elems
    long stat_elemmax;      // list stats
    long stat_elemnow;
    long stat_elemmalloc;
} LIST_t;

#define LISTALLOCNUM 512

elem_t *new_list(LIST_t * LIST, char state);
elem_t *new_tree(LIST_t * LIST, elem_t *key);
elem_t *new_frag(LIST_t * LIST, char state, uint16_t len, unsigned char *frag);
elem_t *new_shortstr(LIST_t * LIST, char state, char *str);
elem_t *ref_list(LIST_t * LIST, elem_t * list);
void append_addref(elem_t * list, elem_t * elem);
void append_transfer(elem_t * list, elem_t * elem);
void remove_next_from_list(LIST_t * LIST, elem_t * list, elem_t *elem);
void free_list(LIST_t * LIST, elem_t * elem);
void free_tree(LIST_t *LIST, elem_t * p);
void free_tree_item(LIST_t *LIST, elem_t * p);

#ifdef __cplusplus
}
#endif

#endif
