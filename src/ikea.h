/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef IKEA_H
#define IKEA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

typedef struct ikea_store_s ikea_store_t;
typedef struct ikea_box_s ikea_box_t;

ikea_store_t * ikea_store_open( const char * oldstore );
void ikea_store_snapshot ( ikea_store_t *ikea_store );
void ikea_store_restore ( ikea_store_t *ikea_store );
void ikea_store_close ( ikea_store_t *ikea_store );

ikea_box_t* ikea_box_open( ikea_store_t *ikea_store, const char *appends_content );
void ikea_box_append( ikea_box_t* ikea_box, const char *data, size_t data_len );
char * ikea_box_close( ikea_box_t* ikea_box );

#ifdef __cplusplus
}
#endif

#endif
