/* vim:set shiftwidth=4 ts=8 expandtab: */

// sizeof(inbuf_t) = 1<<7  (128 bytes)
// the size of .buf is sizeof(inbuf_t) less the other bits  (~115 bytes, I think)
#define INBUFSIZE ((1<<7) - sizeof(inbuf_t*) - sizeof(int) - sizeof(char))
#define INBUFALLOCNUM 128

struct inbuf_s {
    inbuf_t *next;
    int refs;
    unsigned char buf[INBUFSIZE];
    unsigned char end_of_buf;    // maintain a '\0' here 
};

void new_inbuf(context_t * C);
void free_inbuf(context_t * C, inbuf_t * inbuf);
