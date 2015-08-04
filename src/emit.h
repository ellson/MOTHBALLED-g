#define OUT stdout
#define ERR stderr

char *get_name(char *p);
char char_prop(unsigned char prop, char noprop);

typedef struct {
    void (*start_file) (context_t *C);
    void (*sep) (context_t *C);
    void (*start_state) (context_t *C, char class, unsigned char prop, int nest, int repc);
    void (*tree) (context_t *C, elem_t *root);
    void (*string) (context_t *C, elem_t *branch);
    void (*frag) (context_t *C, unsigned char len, unsigned char *frag);
    void (*tok) (context_t *C, char class, unsigned char len, unsigned char *frag);
    void (*end_state) (context_t *C, char class, int rc, int nest, int repc);
    void (*term) (context_t *C);
    void (*end_file) (context_t *C);
    void (*error) (context_t *C, char *message);
} emit_t;


#define emit_start_file(C) \
    if (emit->start_file) {emit->start_file(C);}
#define emit_sep(C) \
    if (emit->sep) {emit->sep(C);}
#define emit_start_state(C, class, prop, nest, repc) \
    if (emit->start_state) {emit->start_state(C, class, prop, nest, repc);}
#define emit_tree(C, root) \
    if (emit->tree) {emit->tree(C, root);}
#define emit_string(C, branch) \
    if (emit->string) {emit->string(C, branch);}
#define emit_frag(C, len, frag) \
    if (emit->frag) {emit->frag(C, len, frag);}
#define emit_tok(C, class, len, frag) \
    if (emit->tok) {emit->tok(C, class, len, frag);}
#define emit_end_state(C, class, rc, nest, repc) \
    if (emit->end_state) {emit->end_state(C, class, rc, nest, repc);}
#define emit_term(C) \
    if (emit->term) {emit->term(C);}
#define emit_end_file(C) \
    if (emit->end_file) {emit->end_file(C);}
#define emit_error(C, message) \
    if (emit->error) {emit->error(C, message);}

extern emit_t *emit,
    *emit_t_api,
    *emit_t_api1,
    *emit_g_api,
    *emit_g_api1;
