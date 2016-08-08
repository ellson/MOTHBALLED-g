/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef enum {
    LISTELEM = 0, // must be 0 for static or calloc allocation of list headers
    FRAGELEM = 1,
    HASHELEM = 2
} elemtype_t;

// Print a list (tree) -  used for debugging
#define P(L) {C->sep = ' ';print_list(stdout, L, 0, &(C->sep));putc('\n', stdout);}

struct elem_s {
    elem_t *next;
    union {
        struct {
            elem_t *first;    // for push, pop, and forward walk
            elem_t *last;    // for append
        } list;
        struct {
            inbuf_t *inbuf;    // inbuf containing frag - for memory management
            unsigned char *frag;    // point to beginning of frag
        } frag;
        struct {
            unsigned long hash; // hash value
            FILE *out;      // file handle, or NULL if not opened yet.
        } hash;
    } u;
    // FIXME - would be better if count was in the union as either "len" or "refs"
    // but doing so results in the size of this struct growing from 32 to 40bytes
    unsigned int count;    // used for list reference count or fragment string length
    // FIXME -- can't store type as elemtype_t, or state as state_t, or the commpiler assumes ints.
    char type;        // LISTELEM, FRAGELEM or HASHELEM
    char state;        // state_machine state that generated this list
};

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))
#define LISTALLOCNUM 512

elem_t *new_hash(context_t * C, unsigned long hash);
elem_t *new_frag(context_t * C, char state, unsigned int len, unsigned char *frag);
elem_t *move_list(context_t * C, elem_t * list);
elem_t *ref_list(context_t * C, elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void free_list(context_t * C, elem_t * list);
int print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int nest, char *sep);
