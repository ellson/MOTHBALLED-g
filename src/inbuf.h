typedef struct inbuf_s inbuf_t;

#define INBUFSIZE ((1<<7) - sizeof(inbuf_t*) - sizeof(char))

struct inbuf_s {
    inbuf_t *next;
    unsigned char buf[INBUFSIZE];
    unsigned char end_of_buf;
};

typedef struct {
    FILE *file;
    char *filename;
    inbuf_t *inbuf;
    int size;
} context_t;

inbuf_t * new_inbuf(void);
unsigned char * more_in(context_t *C);
