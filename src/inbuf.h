typedef struct inbuf_s inbuf_t;

// sizeof(inbuf_t) = 1<<7  (128 bytes)
// the size of .buf is that less the other bits  (~115 bytes, I think)
#define INBUFSIZE ((1<<7) - sizeof(inbuf_t*) - sizeof(int) - sizeof(char))

struct inbuf_s {
    inbuf_t *next;
    int refs;
    unsigned char buf[INBUFSIZE];
    unsigned char end_of_buf;
};

typedef struct {
    char *filename;
    FILE *file;
    inbuf_t *inbuf;
    int size;
    unsigned char *in;
    state_t insi;
} context_t;

typedef struct elem_s elem_t;

// LISTELEM must = 0 for static or calloc allocation of list headers
// FIXME - Should use enum, but don't need an int to store a boolean
#define LISTELEM 0
#define FRAGELEM 1

struct elem_s {
    elem_t *next;
    union {
        struct {
	    elem_t *first; // for prepend and fforward walk
            elem_t *last;  // fpr append
        } list;
        struct {
    	    inbuf_t *inbuf; // inbuf containing frag
	    unsigned char *frag; // point to beginning of frag
        } frag;
    } u;
    // FIXME -- There must be a better way ?
    //          If these ints are included in the above union{}, then
    //          the size of elem_t increases from 32 to 40 bytes.
    union {
        struct {
            int refs;      // reference count
        } list;
        struct {
            int len;       // length of frag
        } frag;
    } v;
    char type;             // LISTELEM or FRAGELEM
    char state;            // state_machine state that generated this list
};

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))

extern long filecount;
extern long actcount;

unsigned char * more_in(context_t *C);

elem_t* new_frag(char state, unsigned char *frag, int len, inbuf_t *inbuf);
elem_t *move_list(char state, elem_t *list);
elem_t *ref_list(char state, elem_t *list);
void append_list(elem_t *list, elem_t *elem);
void free_list(elem_t *list);

void print_frag(FILE* chan, unsigned char len, unsigned char *frag);
int print_len_frag(FILE *chan,unsigned char *len_frag);
void print_list(FILE *chan, elem_t *list, int nest, char sep);
void print_stats(FILE *chan, struct timespec *starttime);
