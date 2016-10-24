/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef enum {
    LISTELEM = 0,           // must be 0 for static or calloc allocation of list headers
    FRAGELEM = 1,
    SHORTSTRELEM = 2,
    HASHNAMEELEM = 3
} elemtype_t;

typedef struct elem_s elem_t;
struct elem_s { 
    elem_t *next;                  // next elem in parent's list
    union {
        struct {
            elem_t *first;         // first elem in this list (or NULL)
            elem_t *last;          // last elem in this list (or NULL)
        } l;
        struct {
            inbuf_t *inbuf;        // inbuf containing frag - for memory management
            unsigned char *frag;   // pointer to beginning of frag
        } f;
        struct {
            unsigned char str[16]; // the short string
        } s;
        struct {
            unsigned char *hashname;// a filename constructed from a hash of the subject
            FILE *out;             // file handle, or NULL if not opened yet.
        } h;
    } u;
    uint16_t unused; 
    uint16_t refs;          // don't free this list until refs == 0
    uint16_t len;           // length of list, frag, or string
    char state;             // state_machine state that generated this elem
    char type;              // LISTELEM
};

typedef struct {
    INBUF_t INBUF;          // Header for inbuf management   
    elem_t *free_elem_list; // linked list of unused list elems
    long stat_elemmax;      // list stats
    long stat_elemnow;
    long stat_elemmalloc;
} LIST_t;

#define LISTALLOCNUM 512

elem_t *new_hashname(LIST_t * LIST, unsigned char *hash, size_t hash_len);
elem_t *new_list(LIST_t * LIST, char state);
elem_t *new_frag(LIST_t * LIST, char state, uint16_t len, unsigned char *frag);
elem_t *new_shortstr(LIST_t * LIST, char state, char *str);
elem_t *move_list(LIST_t * LIST, elem_t * list);
elem_t *ref_list(LIST_t * LIST, elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void remove_next_from_list(LIST_t * LIST, elem_t * list, elem_t *elem);
void free_list(LIST_t * LIST, elem_t * list);
uint16_t print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int nest, char *sep);
