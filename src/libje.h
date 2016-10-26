/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef LIBJE_H
#define LIBJE_H

#ifdef __cplusplus
extern "C" {
#endif

//opaque structs from the public interface
typedef struct context_s CONTEXT_t;

typedef enum {
    SUCCESS,
    FAIL
} success_t;

// libje.c
CONTEXT_t *je_initialize( int *argc, char *argv[], int optind );
void je_finalize( CONTEXT_t * C );
void je_interrupt( CONTEXT_t * C );

// emit.c
success_t je_select_emitter(char *name);

// parse.c
success_t je_parse( CONTEXT_t * C);

// info.c
char * je_session( CONTEXT_t * C );
char * je_stats( CONTEXT_t * C );

// dumpg.c
void set_sstyle( void );
void printg( void );
void dumpg( void );

#ifdef __cplusplus
}
#endif

#endif
