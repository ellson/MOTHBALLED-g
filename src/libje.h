typedef struct context_s context_t;
typedef struct elem_s elem_t;
typedef struct emit_s emit_t;

#define SIZEOF_EMITTERS 6
extern emit_t *emitters[];

typedef enum {
    SUCCESS,
    FAIL
} success_t;

// libje.c
context_t *je_initialize(void);
void je_finalize(context_t *C);

// persist.c
elem_t * je_persist_open(context_t *C);
void je_persist_snapshot (context_t *C);
void je_persist_restore (context_t *C);
void je_persist_close (context_t *C);

// parse.c
success_t je_parse(context_t * C, elem_t *name);

// info.c
char * je_session(context_t *C);
char * je_stats(context_t *C);

// dumpg.c
void set_sstyle(void);
void printg(void);
void dumpg(void);

