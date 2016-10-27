/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef LIBJE_H
#define LIBJE_H

#ifdef __cplusplus
extern "C" {
#endif

//opaque structs from the public interface
typedef struct parse_s PARSE_t;

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
PARSE_t *initialize( int *argc, char *argv[], int optind );
void finalize( PARSE_t * PARSE );
void interrupt( PARSE_t * PARSE );

// emit.c
success_t select_emitter(char *name);

// parse.c
success_t parse( PARSE_t * PARSE);

// info.c
char * session( PARSE_t * PARSE );
char * stats( PARSE_t * PARSE );

// dumpg.c
void set_sstyle( void );
void printg( void );
void dumpg( void );

#ifdef __cplusplus
}
#endif

#endif
