/* vim:set shiftwidth=4 ts=8 expandtab: */

#include <stdint.h>

#include "grammar.h"
#include "fatal.h"
#include "inbuf.h"
#include "list.h"
#include "token.h"

//opaque structs from the public interface
typedef struct context_s CONTEXT_t;

// libje.c
CONTEXT_t *je_initialize( int *argc, char *argv[], int optind );
void je_finalize( CONTEXT_t * C );
void je_interrupt( CONTEXT_t * C );

// emit.c
success_t je_select_emitter(char *name);

// parse.c
success_t je_parse( CONTEXT_t * C, elem_t *name );

// info.c
char * je_session( CONTEXT_t * C );
char * je_stats( CONTEXT_t * C );

// dumpg.c
void set_sstyle( void );
void printg( void );
void dumpg( void );

