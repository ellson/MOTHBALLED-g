typedef struct elem_s elem_t;

// LST must = 0 for calloc of list headers
typedef enum {LISTELEM, FRAGELEM} elemtype_t;

struct elem_s {
    elem_t *next;
    int state;
    elemtype_t type;
    union {
        struct {
	    elem_t *first; // for prepend and fforward walk
            elem_t *last;  // fpr append
        } list;
	struct {
	    unsigned char *frag;
            int len;
    	    int allocated;  // FIXME - for buffer nabagement
        } frag;
    } u;
};

elem_t* new_list(int state);
elem_t* new_frag(int state, unsigned char *frag, int len, int allocated);
elem_t *list2elem(elem_t *list);
void prepend_list(elem_t *list, elem_t *elem);
void append_list(elem_t *list, elem_t *elem);
void free_list(elem_t *list);
void print_list(FILE *chan, elem_t *list);
