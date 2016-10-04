/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef struct elem_s elem_t;
typedef struct frag_elem_s frag_elem_t;
typedef struct hash_elem_s hash_elem_t;
typedef struct hashname_elem_s hashname_elem_t;

typedef enum {
    LISTELEM = 0, // must be 0 for static or calloc allocation of list headers
    FRAGELEM = 1,
    HASHELEM = 2,
    HASHNAMEELEM = 3
} elemtype_t;

// Print a list (tree) -  used for debugging
#define P(L) {C->sep = ' ';print_list(stdout, L, 0, &(C->sep));putc('\n', stdout);}

typedef struct LISTS_s LISTS_t;

struct LISTS_s {
    INBUFS_t INBUFS;        // Header for inbug management   
    elem_t *free_elem_list;    // linked list of unused list elems
    long stat_elemmalloc;
    long stat_elemmax;
    long stat_elemnow;
};

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
    uint64_t hash;       // hash value
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

elem_t *new_hash(LISTS_t * LISTS, uint64_t hash);
elem_t *new_hashname(LISTS_t * LISTS, unsigned char *hash, size_t hash_len);
elem_t *new_frag(LISTS_t * LISTS, char state, unsigned int len, unsigned char *frag);
elem_t *move_list(LISTS_t * LISTS, elem_t * list);
elem_t *ref_list(LISTS_t * LISTS, elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void free_list(LISTS_t * LISTS, elem_t * list);
int print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int nest, char *sep);
