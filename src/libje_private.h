/* vim:set shiftwidth=4 ts=8 expandtab: */

#ifndef LIBJE_PRIVATE_H
#define LIBJE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

// include local configuration
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
typedef struct ikea_store_s ikea_store_t;
typedef struct ikea_box_s ikea_box_t;

// private headers
#include "hash.h"
#include "ikea.h"
#include "context.h"
#include "emit.h"
#include "pattern.h"
#include "sameas.h"
#include "dispatch.h"
#include "expand.h"

#ifdef __cplusplus
}
#endif

#endif
