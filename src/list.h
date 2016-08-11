/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef enum {
    LISTELEM = 0, // must be 0 for static or calloc allocation of list headers
    FRAGELEM = 1,
    HASHELEM = 2,
    HASHNAMEELEM = 3
} elemtype_t;

// Print a list (tree) -  used for debugging
#define P(L) {C->sep = ' ';print_list(stdout, L, 0, &(C->sep));putc('\n', stdout);}

struct elem_s {          // castable from frag_elem_s and hash_elem_s -- sizes must match (32bytes)
    elem_t *next;
    elem_t *first;
    elem_t *last;
    unsigned int refs;
    char type;           // LISTELEM
    char state;          // state_machine state that generated this list
};

struct frag_elem_s {     // castable to elem_s  -- size must match (32bytes)
    frag_elem_t *next;
    inbuf_t *inbuf;      // inbuf containing frag - for memory management
    unsigned char *frag; // pointer to beginning of frag
    unsigned int len;    // length of frag
    char type;           // FRAGELEM
    char state;          // state_machine state that generated this list
};

struct hash_elem_s {     // castable to elem_s  -- size must match (32bytes)
    hash_elem_t *next;
    unsigned long hash;  // hash value
    FILE *out;           // file handle, or NULL if not opened yet.
    unsigned int count;  // unused
    char type;           // HASHELEM
    char state;          // state_machine state that generated this list
};

struct hashname_elem_s {     // castable to elem_s  -- size must match (32bytes)
    hash_elem_t *next;
    char *hashname;      // a filename constructed from a hash of the subject
    FILE *out;           // file handle, or NULL if not opened yet.
    unsigned int count;  // unused
    char type;           // HASHNAMEELEM
    char state;          // unused
};

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))
#define LISTALLOCNUM 512

elem_t *new_hash(context_t * C, unsigned long hash);
elem_t *new_hashname(context_t * C, unsigned char *hash, size_t hash_len);
elem_t *new_frag(context_t * C, char state, unsigned int len, unsigned char *frag);
elem_t *move_list(context_t * C, elem_t * list);
elem_t *ref_list(context_t * C, elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void free_list(context_t * C, elem_t * list);
int print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int nest, char *sep);
