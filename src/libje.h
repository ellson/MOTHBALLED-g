typedef struct inbuf_s inbuf_t;
typedef struct elem_s elem_t;
typedef struct emit_s emit_t;
typedef struct context_s context_t;
typedef struct container_context_s container_context_t;
typedef struct hashfile_s hashfile_t;

context_t *je_initialize(void);
void je_finalize(context_t *C);
