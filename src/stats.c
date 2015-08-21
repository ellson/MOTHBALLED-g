#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "list.h"
#include "stats.h"

#define STAT_FORMAT "%18s = %ld"

#define TEN9 1000000000

long stat_filecount;
long stat_lfcount;
long stat_crcount;
long stat_inchars;
long stat_actcount;
long stat_patterncount;
long stat_containercount;
long stat_stringcount;
long stat_fragcount;
long stat_inbufmalloc;
long stat_inbufmax;
long stat_inbufnow;
long stat_elemmalloc;
long stat_elemmax;
long stat_elemnow;


// FIXME - needs to be dynamically built on each viewing.
//       - build in minimal-space g format,  then use pretty-printer
//         for output.
//       - same pretty-printer can be used for session

void print_stats(FILE * chan, struct timespec *starttime)
{
	success_t rc;
	struct timespec nowtime;
	long runtime;

	rc = clock_gettime(CLOCK_MONOTONIC_RAW, &nowtime);
	assert(rc == SUCCESS);

	runtime = (nowtime.tv_sec * TEN9 + nowtime.tv_nsec)
	    - (starttime->tv_sec * TEN9 + starttime->tv_nsec);

	fprintf(chan, "\n");
	fprintf(chan, "stats [\n");
	fprintf(chan, STAT_FORMAT ".%09ld\n", "runtime", runtime / TEN9,
		runtime % TEN9);
	fprintf(chan, STAT_FORMAT "\n", "files", stat_filecount);
	fprintf(chan, STAT_FORMAT "\n", "lines",
		1 + (stat_lfcount ? stat_lfcount : stat_crcount));
	fprintf(chan, "\n");
	fprintf(chan, STAT_FORMAT "\n", "acts", stat_actcount);
	fprintf(chan, STAT_FORMAT "\n", "acts_per_second",
		stat_actcount * TEN9 / runtime);
	fprintf(chan, STAT_FORMAT "\n", "patterns", stat_patterncount);
	fprintf(chan, STAT_FORMAT "\n", "containers", stat_containercount);
	fprintf(chan, STAT_FORMAT "\n", "strings", stat_stringcount);
	fprintf(chan, STAT_FORMAT "\n", "fragments", stat_fragcount);
	fprintf(chan, STAT_FORMAT "\n", "input_chars", stat_inchars);
	fprintf(chan, STAT_FORMAT "\n", "chars_per_second",
		stat_inchars * TEN9 / runtime);
	fprintf(chan, "\n");
	fprintf(chan, STAT_FORMAT "\n", "inbufsize", sizeof(inbuf_t));
	fprintf(chan, STAT_FORMAT "\n", "inbufmax", stat_inbufmax);
	fprintf(chan, STAT_FORMAT "\n", "inbufnow", stat_inbufnow);
	fprintf(chan, "\n");
	fprintf(chan, STAT_FORMAT "\n", "elemsize", size_elem_t);
	fprintf(chan, STAT_FORMAT "\n", "elemmax", stat_elemmax);
	fprintf(chan, STAT_FORMAT "\n", "elemnow", stat_elemnow);
	fprintf(chan, "\n");
	fprintf(chan, STAT_FORMAT "\n", "inbufmallocsize",
		INBUFALLOCNUM * sizeof(inbuf_t));
	fprintf(chan, STAT_FORMAT "\n", "inbufmalloccount", stat_inbufmalloc);
	fprintf(chan, STAT_FORMAT "\n", "inbufmalloctotal",
		stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t));
	fprintf(chan, "\n");
	fprintf(chan, STAT_FORMAT "\n", "elemmallocsize",
		LISTALLOCNUM * size_elem_t);
	fprintf(chan, STAT_FORMAT "\n", "elemmalloccount", stat_elemmalloc);
	fprintf(chan, STAT_FORMAT "\n", "elemmalloctotal",
		stat_elemmalloc * LISTALLOCNUM * size_elem_t);
	fprintf(chan, "\n");
	fprintf(chan, STAT_FORMAT "\n", "malloctotal",
		stat_elemmalloc * LISTALLOCNUM * size_elem_t +
		stat_inbufmalloc * INBUFALLOCNUM * sizeof(inbuf_t));
	fprintf(chan, "]\n");
}
