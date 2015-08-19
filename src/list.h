typedef struct elem_s elem_t;

// LISTELEM must = 0 for static or calloc allocation of list headers
typedef enum {
	LISTELEM = 0,
	FRAGELEM = 1
} elemtype_t;

struct elem_s {
	elem_t *next;
	union {
		struct {
			elem_t *first;	// for push, pop, and forward walk
			elem_t *last;	// for append
		} list;
		struct {
			inbuf_t *inbuf;	// inbuf containing frag
			unsigned char *frag;	// point to beginning of frag
		} frag;
	} u;
	// FIXME -- There must be a better way ?
	//          If these "ref" or "len" ints are included in the above union{}, then
	//          the size of elem_t increases from 32 to 40 bytes.
	union {
		struct {
			int refs;	// reference count
		} list;
		struct {
			int len;	// length of frag
		} frag;
	} v;
	// FIXME -- can't store type as elemtype_t or it would assume int.
	char type;		// LISTELEM or FRAGELEM
	char state;		// state_machine state that generated this list
};

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))
#define LISTALLOCNUM 512

elem_t *new_frag(char state, int len, unsigned char *frag, inbuf_t * inbuf);
elem_t *move_list(elem_t * list);
elem_t *ref_list(elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void push_list(elem_t * list, elem_t * elem);
void pop_list(elem_t * list);
void free_list(elem_t * list);

void print_frag(FILE * chan, unsigned char len, unsigned char *frag);
int print_len_frag(FILE * chan, unsigned char *len_frag);
void print_list(FILE * chan, elem_t * list, int nest, char sep);
