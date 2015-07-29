typedef struct elem_s elem_t;

// LST must = 0 for calloc of list headers
typedef enum {LIST, FRAG} elemtype_t;

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

elem_t* newlist(int state);
elem_t* newfrag(int state, unsigned char *frag, int len, int allocated);
elem_t *list2elem(elem_t *list);
void prependlist(elem_t *list, elem_t *elem);
void appendlist(elem_t *list, elem_t *elem);
void freelist(elem_t *list);
void printj(elem_t *list);
