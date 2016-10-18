/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef enum {
    LISTELEM = 0, // must be 0 for static or calloc allocation of list headers
    FRAGELEM = 1,
    SHORTSTRELEM = 2,
    HASHNAMEELEM = 3
} elemtype_t;

typedef struct elem_s elem_t;
struct elem_s {          // castable from frag_elem_s and shortstr_elem_s -- sizes must match (32bytes)
    elem_t *next;
    elem_t *first;
    elem_t *last;
    unsigned int refs;
    char type;           // LISTELEM
    char state;          // state_machine state that generated this list
};

typedef struct frag_elem_s frag_elem_t;
struct frag_elem_s {     // castable to elem_s  -- size must match (32bytes)
    frag_elem_t *next;
    inbuf_t *inbuf;      // inbuf containing frag - for memory management
    unsigned char *frag; // pointer to beginning of frag
    unsigned int len;    // length of frag
    char type;           // FRAGELEM
    char state;          // state_machine state that generated this list
};

typedef struct shortstr_elem_s shortstr_elem_t;
struct shortstr_elem_s {     // castable to elem_s  -- size must match (32bytes)
    shortstr_elem_t *next;
    unsigned int len;    // length of shortstr
    char type;           // SHORTSTRELEM
    char state;          // state_machine state that generated this list
    unsigned char str[18];
};

typedef struct hashname_elem_s hashname_elem_t;
struct hashname_elem_s {     // castable to elem_s  -- size must match (32bytes)
    hashname_elem_t *next;
    char *hashname;      // a filename constructed from a hash of the subject
    FILE *out;           // file handle, or NULL if not opened yet.
    unsigned int count;  // unused
    char type;           // HASHNAMEELEM
    char state;          // unused
};

typedef struct {
    INBUF_t INBUF;        // Header for inbuf management   
    elem_t *free_elem_list;    // linked list of unused list elems
    long stat_elemmalloc;  // list stats
    long stat_elemmax;
    long stat_elemnow;
} LIST_t;

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))
#define LISTALLOCNUM 512

elem_t *new_shortsrt(LIST_t * LIST, unsigned char *str);
elem_t *new_hashname(LIST_t * LIST, unsigned char *hash, size_t hash_len);
elem_t *new_list(LIST_t * LIST, char state);
elem_t *new_frag(LIST_t * LIST, char state, unsigned int len, unsigned char *frag);
elem_t *move_list(LIST_t * LIST, elem_t * list);
elem_t *ref_list(LIST_t * LIST, elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void remove_next_from_list(LIST_t * LIST, elem_t * list, elem_t *elem);
void free_list(LIST_t * LIST, elem_t * list);
int print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int nest, char *sep);
