typedef struct elem_s elem_t;

// LST must = 0 for calloc of list headers
typedef enum {LST, STR} elemtype_t;

struct elem_s {
    elem_t *next;
    int state;
    elemtype_t type;
    union {
        struct {
	    elem_t *first;
            elem_t *last;
        } lst;
	struct {
	    unsigned char *buf;
            int len;
    	    int allocated;
        } str;
    } u;
};

elem_t* newlist(int state);
elem_t* newelem(int state, unsigned char *buf, int len, int allocated);
elem_t *list2elem(elem_t *list);
void appendlist(elem_t *list, elem_t *elem);
void freelist(elem_t *list);
void printj(elem_t *list);
