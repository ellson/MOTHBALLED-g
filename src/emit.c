#include <stdio.h>

#include "emit.h"
#include "grammar.h"

#define OUT stdout
#define ERR stderr

#define PROPP(p) (state_props + (p - state_machine))

static char *get_name(char *p) {
    int offset;
    while (*p) p++;
    offset = *PROPP(p) * 2;
    return state_names + offset;
}

void emit_start_act(context_t *context) {
}

void emit_start_state(context_t *context, char *p) {
    fprintf(OUT,"%s ", get_name(p));
}
