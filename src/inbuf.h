/* vim:set shiftwidth=4 ts=8 expandtab: */

// My guess is that INBUFIZE should match the average ACT size, so that inbufs get released
// roughly as ACTs are processed.    I could be wrong - may need experimentation.
//
// patterns will hold inbufs for a long lime,  so inbufs shouldn't be much bigger than
// the average pattern.
//
// small inbufs cause greater fragmentation of strings
// small inbufs have greater overhead (13bytes + ~1 elem_t (64bytes) per buff for fragmentation)

// sizeof(inbuf_t) = 1<<7  (128 bytes)
// the contents size is sizeof(inbuf_t) less the other bits  (~115 bytes)
#define INBUFIZE ((1<<7) - sizeof(inbuf_t*) - sizeof(int) - sizeof(char))
#define INBUFALLOCNUM 128

typedef struct inbuf_s inbuf_t;
struct inbuf_s {
    inbuf_t *next;
    int refs;
    unsigned char buf[INBUFIZE];
    unsigned char end_of_buf;    // maintain a '\0' here 
};

typedef struct {
    inbuf_t *inbuf;            // the active input buffer
    inbuf_t *free_inbuf_list;  // linked list of unused inbufs
    long stat_inbufmalloc;
    long stat_inbufmax;
    long stat_inbufnow;
} INBUF_t;

inbuf_t * new_inbuf(INBUF_t * INBUF);
void free_inbuf(INBUF_t * INBUF, inbuf_t * inbuf);
