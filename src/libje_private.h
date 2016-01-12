/* vim:set shiftwidth=4 ts=8 expandtab: */

// include common headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <assert.h>

// include local configuration
 # include "config.h"

#if ! defined(HAVE_CLOCK_GETTIME)
#include <sys/time.h>
#endif

// include public interface
#include "libje.h"

// private types that maybe forward referenced
typedef struct inbuf_s inbuf_t;
typedef struct emit_s emit_t;
typedef struct container_context_s container_context_t;
typedef struct hashfile_s hashfile_t;

// private headers
#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "hash.h"
#include "emit.h"
#include "token.h"
#include "context.h"
#include "pattern.h"
#include "sameas.h"
#include "dispatch.h"
#include "expand_edge.h"
