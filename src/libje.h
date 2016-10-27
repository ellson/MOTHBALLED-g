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
PARSE_t *je_initialize( int *argc, char *argv[], int optind );
void je_finalize( PARSE_t * PARSE );
void je_interrupt( PARSE_t * PARSE );

// emit.c
success_t je_select_emitter(char *name);

// parse.c
success_t je_parse( PARSE_t * PARSE);

// info.c
char * je_session( PARSE_t * PARSE );
char * je_stats( PARSE_t * PARSE );

// dumpg.c
void set_sstyle( void );
void printg( void );
void dumpg( void );

#ifdef __cplusplus
}
#endif

#endif
