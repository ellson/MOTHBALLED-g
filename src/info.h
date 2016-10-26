/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef HASH_H
#define HASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>

#include "context.h"
#include "emit.h"

char * je_session(CONTEXT_t *C);

#ifdef __cplusplus
}
#endif

#endif
