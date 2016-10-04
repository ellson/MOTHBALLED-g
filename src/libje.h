/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdint.h>

#include "grammar.h"
#include "fatal.h"
#include "inbuf.h"
#include "list.h"

//opaque structs from the public interface
typedef struct context_s context_t;

typedef enum {
    SUCCESS,
    FAIL
} success_t;

// libje.c
context_t *je_initialize( int *argc, char *argv[], int optind );
void je_finalize( context_t * C );
void je_interrupt( context_t * C );

// emit.c
success_t je_select_emitter(char *name);

// parse.c
success_t je_parse( context_t * C, elem_t *name );

// info.c
char * je_session( context_t * C );
char * je_stats( context_t * C );

// dumpg.c
void set_sstyle( void );
void printg( void );
void dumpg( void );

