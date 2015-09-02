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

#include "grammar.h"
#include "libje.h"

typedef struct inbuf_s inbuf_t;
typedef struct emit_s emit_t;
typedef struct container_context_s container_context_t;
typedef struct hashfile_s hashfile_t;

#include "inbuf.h"
#include "list.h"
#include "hash.h"
#include "emit.h"
#include "token.h"
#include "context.h"
#include "pattern.h"
#include "sameas.h"
#include "dispatch.h"
