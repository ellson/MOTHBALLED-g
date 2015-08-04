typedef struct inbuf_s inbuf_t;

#define INBUFSIZE ((1<<7) - sizeof(inbuf_t*) - sizeof(char))

struct inbuf_s {
    inbuf_t *next;
    unsigned char buf[INBUFSIZE];
    unsigned char end_of_buf;
};

inbuf_t* new_inbuf(void);
