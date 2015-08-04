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
	    unsigned char *frag;
    	    void *allocated;  // FIXME - for buffer nabagement
        } frag;
    } u;
    char type; // LISTELEM or FRAGELEM
    char state;  // state_machine state that generated the list
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
