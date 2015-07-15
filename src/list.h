typedef struct elem_s elem_t;

struct elem_s {
    int type;
    unsigned char *buf;
    int len;
    int allocated;
    elem_t *next;
};

elem_t* newelem(int type, unsigned char *buf, int len, int allocated, elem_t *next);
elem_t* joinlist2elem(elem_t *list, int type, elem_t *next);
void freelist(elem_t *list);
void freefreelist(void);

