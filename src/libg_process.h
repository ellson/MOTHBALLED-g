/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef LIBG_PROCESS_H
#define LIBG_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

// session.c
void session(int *pargc, char *argv[], int optind, char needstats);

// emit.c
int select_emitter(char *name);

// dumpg.c
void set_sstyle( void );
void printg( void );
void dumpg( void );

#ifdef __cplusplus
}
#endif

#endif
