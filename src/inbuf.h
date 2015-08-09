typedef struct inbuf_s inbuf_t;

// sizeof(inbuf_t) = 1<<7  (128 bytes)
// the size of .buf is sizeof(inbuf_t) less the other bits  (~115 bytes, I think)
#define INBUFSIZE ((1<<7) - sizeof(inbuf_t*) - sizeof(int) - sizeof(char))
#define INBUFALLOCNUM 128

struct inbuf_s {
    inbuf_t *next;
    int refs;
    unsigned char buf[INBUFSIZE];
    unsigned char end_of_buf;      // maintain a '\0' here 
};

typedef struct elem_s elem_t;

// LISTELEM must = 0 for static or calloc allocation of list headers
typedef enum {
    LISTELEM=0,
    FRAGELEM=1
} elemtype_t;

struct elem_s {
    elem_t *next;
    union {
        struct {
	    elem_t *first; // for push, pop, and forward walk
            elem_t *last;  // for append
        } list;
        struct {
    	    inbuf_t *inbuf; // inbuf containing frag
	    unsigned char *frag; // point to beginning of frag
        } frag;
    } u;
    // FIXME -- There must be a better way ?
    //          If these "ref" or "len" ints are included in the above union{}, then
    //          the size of elem_t increases from 32 to 40 bytes.
    union {
        struct {
            int refs;      // reference count
        } list;
        struct {
            int len;       // length of frag
        } frag;
    } v;
    // FIXME -- can't store type as elemtype_t or it would assume int.
    char type;             // LISTELEM or FRAGELEM
    char state;            // state_machine state that generated this list
};

typedef struct {
    char *filename;        // name of file currently being processed, or "-" for stdin
    FILE *file;            // open file handle for file currently being processed
    inbuf_t *inbuf;        // the active input buffer
    unsigned char *in;     // next charater to be processed
    state_t insi;          // state represented by last character read
    elem_t subject;        // header of subject stack for containment
			   //  (Current subject is first in list.  Parents follow.)
    elem_t sameend_legs;   // header of list of LEGS from previous ACT.
    elem_t sameend_legs_new; // header of accumulating list of LEGS in current ACT.
} context_t;

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))
#define LISTALLOCNUM 512

success_t more_in(context_t *C);

elem_t* new_frag(char state, unsigned char *frag, int len, inbuf_t *inbuf);
elem_t *move_list(char state, elem_t *list);
elem_t *ref_list(char state, elem_t *list);
void append_list(elem_t *list, elem_t *elem);
void push_list(elem_t *list, elem_t *elem);
void pop_list(elem_t *list);
void free_list(elem_t *list);

void print_frag(FILE* chan, unsigned char len, unsigned char *frag);
int print_len_frag(FILE *chan,unsigned char *len_frag);
void print_list(FILE *chan, elem_t *list, int nest, char sep);
void print_stats(FILE *chan, struct timespec *starttime);
