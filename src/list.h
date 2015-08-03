typedef struct elem_s elem_t;

// LST must = 0 for calloc of list headers
typedef enum {
	LISTELEM=0,     // a list elem managed by new_list() and free_list()
	FRAGELEM        // a frag elem managed by new_frag() and free_list()
} elemtype_t;

struct elem_s {
    elem_t *next;
    union {
        struct {
	    elem_t *first; // for prepend and fforward walk
            elem_t *last;  // fpr append
        } list;
        struct {
	    unsigned char *frag;
    	    void *allocated;  // FIXME - for buffer nabagement
        } frag;
    } u;
    elemtype_t type;
    char state;
    int len; // length of frag, or length of all frags in a list
};

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))

elem_t* new_frag(char state, unsigned char *frag, int len, void *allocated);
elem_t *list2elem(elem_t *list, int len);
void prepend_list(elem_t *list, elem_t *elem);
void append_list(elem_t *list, elem_t *elem);
void free_list(elem_t *list);
int print_string(FILE *chan,unsigned char *len_frag);
void print_frag(FILE* chan, unsigned char len, unsigned char *frag);
void print_list(FILE *chan, elem_t *list, int nest, char sep);
