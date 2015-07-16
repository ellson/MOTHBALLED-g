typedef struct elem_s elem_t;

struct elem_s {
    int type;
    unsigned char *buf;
    int len;
    int allocated;
    elem_t *next;
};

typedef struct {
    int type;
    elem_t *first, *last;
} elemlist_t;

elem_t* newelem(int type, unsigned char *buf, int len, int allocated);
elem_t* joinlist2elem(elemlist_t *list, int type);
void appendlist(elemlist_t *list, elem_t *elem);
void freelist(elemlist_t *list);
void freefreelist(void);
void printj(elemlist_t *list, char *join);

