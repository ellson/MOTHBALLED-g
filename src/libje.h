/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef LIBJE_H
#define LIBJE_H

#ifdef __cplusplus
extern "C" {
#endif

//opaque structs from the public interface
typedef struct parse_s PARSE_t;
typedef struct session_s SESSION_t;

typedef enum {
    SUCCESS,
    FAIL
} success_t;

// emit styles
typedef enum {
    MINIMUM_SPACE_STYLE = 0,
    SHELL_FRIENDLY_STYLE = 1
} style_t;

// libje.c
SESSION_t *initialize( int *argc, char *argv[], int optind );
void finalize( SESSION_t * SESSION );
void interrupt( SESSION_t * SESSION );
success_t parse(SESSION_t * SESSION);

// emit.c
success_t select_emitter(char *name);

// info.c
char * session( SESSION_t * SESSION );
char * stats( SESSION_t * SESSION );

// dumpg.c
void set_sstyle( void );
void printg( void );
void dumpg( void );

#ifdef __cplusplus
}
#endif

#endif
