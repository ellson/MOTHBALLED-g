typedef struct elem_s elem_t;

typedef enum {STR, LST} elemtype_t;

struct elem_s {
    elem_t *next;
    int props;
    elemtype_t type;
    union {
	struct {
	    unsigned char *buf;
            int len;
    	    int allocated;
        } str;
        struct {
	    elem_t *first;
            elem_t *last;
        } lst;
    } u;
};


elem_t* newlist(int props);
elem_t* newelem(int props, unsigned char *buf, int len, int allocated);
elem_t *list2elem(elem_t *list);
void appendlist(elem_t *list, elem_t *elem);
void freelist(elem_t *list);
void printj(elem_t *list);

