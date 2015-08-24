typedef struct elem_s elem_t;
typedef struct context_s context_t;

// LISTELEM must = 0 for static or calloc allocation of list headers
typedef enum {
	LISTELEM = 0,
	FRAGELEM = 1,
	HASHELEM = 2
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
		struct {
			unsigned long hash; // hash value
			FILE *out;      // file handle, or NULL if not opened yet.
		} hash;
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
	// FIXME -- can't store type as elemtype_t, or state as state_t, or it would assume ints.
	char type;		// LISTELEM or FRAGELEM
	char state;		// state_machine state that generated this list
};

#define size_elem_t (sizeof(elem_t*)*((sizeof(elem_t)+sizeof(elem_t*)-1)/(sizeof(elem_t*))))
#define LISTALLOCNUM 512

elem_t *new_hash(context_t * C, unsigned long hash);
elem_t *new_frag(context_t * C, char state, int len, unsigned char *frag);
elem_t *move_list(context_t * C, elem_t * list);
elem_t *ref_list(context_t * C, elem_t * list);
void append_list(elem_t * list, elem_t * elem);
void free_list(context_t * C, elem_t * list);

int print_len_frag(FILE * chan, unsigned char *len_frag);
void print_frags(FILE * chan, state_t liststate, elem_t * elem, char *sep);
void print_list(FILE * chan, elem_t * list, int nest, char *sep);
