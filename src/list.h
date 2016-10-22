/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef enum {
    LISTELEM = 0,           // must be 0 for static or calloc allocation of list headers
    FRAGELEM = 1,
    SHORTSTRELEM = 2,
    HASHNAMEELEM = 3
} elemtype_t;

typedef struct elem_s elem_t;
struct elem_s {             // castable from frag_elem_s and shortstr_elem_s
                            // -- sizes must match (32bytes)
    elem_t *next;
    elem_t *first;
    elem_t *last;
    uint16_t unused[2];
    uint16_t refs;
    char state;             // state_machine state that generated this list
    char type;              // LISTELEM
};

typedef struct frag_elem_s frag_elem_t;
struct frag_elem_s {        // castable to elem_s  -- size must match (32bytes)
    frag_elem_t *next;
    inbuf_t *inbuf;         // inbuf containing frag - for memory management
    unsigned char *frag;    // pointer to beginning of frag
    uint16_t unused[2];
    uint16_t len;           // length of frag
    char state;             // state_machine state that generated this list
    char type;              // LISTELEM
};

typedef struct shortstr_elem_s shortstr_elem_t;
struct shortstr_elem_s {    // castable to elem_s  -- size must match (32bytes)
    shortstr_elem_t *next;
    unsigned char str[20];
    uint16_t len;           // length of shortstr
    char state;             // state_machine state that generated this list
    char type;              // SHORTSTRELEM
};

typedef struct hashname_elem_s hashname_elem_t;
struct hashname_elem_s {    // castable to elem_s  -- size must match (32bytes)
    hashname_elem_t *next;
    unsigned char *hashname;// a filename constructed from a hash of the subject
    FILE *out;              // file handle, or NULL if not opened yet.
    uint16_t unused[3];
    char state;             // state_machine state that generated this list
    char type;              // HASHNAMEELEM
};

typedef struct {
    INBUF_t INBUF;          // Header for inbuf management   
    elem_t *free_elem_list; // linked list of unused list elems
    long stat_elemmax;      // list stats
    long stat_elemnow;
    long stat_elemmalloc;
} LIST_t;

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))
#define LISTALLOCNUM 512

elem_t *new_hashname(LIST_t * LIST, unsigned char *hash, size_t hash_len);
elem_t *new_list(LIST_t * LIST, char state);
elem_t *new_frag(LIST_t * LIST, char state, uint16_t len, unsigned char *frag);
elem_t *new_shortstr(LIST_t * LIST, char state, unsigned char *str);
elem_t *move_list(LIST_t * LIST, elem_t * list);
elem_t *ref_list(LIST_t * LIST, elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void remove_next_from_list(LIST_t * LIST, elem_t * list, elem_t *elem);
void free_list(LIST_t * LIST, elem_t * list);
uint16_t print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int nest, char *sep);
