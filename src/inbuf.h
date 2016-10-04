/* vim:set shiftwidth=4 ts=8 expandtab: */

typedef struct inbuf_s inbuf_t;
typedef struct INBUFS_s INBUFS_t;

// My guess is that INBUFSIZE should match the average ACT size, so that inbufs get released
// roughly as ACTs are processed.    I could be wrong - may need experimentation.
//
// patterns will hold inbufs for a long lime,  so shouldn't be much bigger that the average pattern.
//
// small buffer cause greater fragmentation of strings
// small buffers have greater overhead (13bytes + ~1 elem_t (64bytes) per buff for fragmentation)

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

struct INBUFS_s {
    inbuf_t *inbuf;            // the active input buffer
    inbuf_t *free_inbuf_list;  // linked list of unused inbufs
    long stat_inbufmalloc;
    long stat_inbufmax;
    long stat_inbufnow;
};

inbuf_t * new_inbuf(INBUFS_t * INBUFS);
void free_inbuf(INBUFS_t * INBUFS, inbuf_t * inbuf);
