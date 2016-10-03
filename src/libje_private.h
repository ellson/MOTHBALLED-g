/* vim:set shiftwidth=4 ts=8 expandtab: */

// include local configuration
#include "config.h"

#ifdef HAVE_SYSINFO
#include <sys/sysinfo.h>
#else
#ifndef HAVE_CLOCK_GETTIME
#include <sys/time.h>
#endif
#endif

// include public interface
#include "libje.h"

// private types that maybe forward referenced
typedef struct emit_s emit_t;
typedef struct container_context_s container_context_t;
typedef struct ikea_store_s ikea_store_t;
typedef struct ikea_box_s ikea_box_t;

// private headers
#include "grammar.h"
#include "inbuf.h"
#include "fatal.h"
#include "list.h"
#include "hash.h"
#include "ikea.h"
#include "emit.h"
#include "token.h"
#include "context.h"
#include "pattern.h"
#include "sameas.h"
#include "dispatch.h"
#include "expand_edge.h"
