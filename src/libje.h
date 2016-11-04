/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef LIBJE_H
#define LIBJE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "success.h"
#include "thread.h"

// libje.c
THREAD_t *initialize( int *argc, char *argv[], int optind );
void finalize( THREAD_t * THREAD );
void interrupt( THREAD_t * THREAD );
success_t parse(THREAD_t * THREAD);

// emit.c
success_t select_emitter(char *name);

// info.c
char * session( THREAD_t * THREAD );
char * stats( THREAD_t * THREAD );

// dumpg.c
void set_sstyle( void );
void printg( void );
void dumpg( void );

#ifdef __cplusplus
}
#endif

#endif
