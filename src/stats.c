#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "grammar.h"
#include "inbuf.h"
#include "stats.h"

#define STAT_FORMAT "%18s=%ld"

#define TEN9 1000000000

long stat_filecount;
long stat_inchars;
long stat_actcount;
long stat_containercount;
long stat_inbufmalloc;
long stat_inbufmax;
long stat_inbufnow;
long stat_elemmalloc;
long stat_elemmax;
long stat_elemnow;

void print_stats(FILE *chan, struct timespec *starttime) {
    int rc;
    struct timespec nowtime;
    long runtime;

    rc = clock_gettime(CLOCK_MONOTONIC_RAW, &nowtime);
    assert(rc == 0);

    runtime = (nowtime.tv_sec * TEN9 + nowtime.tv_nsec)
                - (starttime->tv_sec * TEN9 + starttime->tv_nsec);
    
    fprintf(chan,"\n");
    fprintf(chan,"stats [\n");
    fprintf(chan,STAT_FORMAT".%09ld\n", "runtime",     runtime / TEN9, runtime % TEN9);
    fprintf(chan,STAT_FORMAT"\n", "files",             stat_filecount);
    fprintf(chan,"\n");
    fprintf(chan,STAT_FORMAT"\n", "input_chars",       stat_inchars);
    fprintf(chan,STAT_FORMAT"\n", "chars_per_second",  stat_inchars * TEN9 / runtime);
    fprintf(chan,"\n");
    fprintf(chan,STAT_FORMAT"\n", "acts",              stat_actcount);
    fprintf(chan,STAT_FORMAT"\n", "acts_per_second",   stat_actcount * TEN9 / runtime);
    fprintf(chan,"\n");
    fprintf(chan,STAT_FORMAT"\n", "containers",        stat_containercount);
    fprintf(chan,"\n");
    fprintf(chan,STAT_FORMAT"\n", "inbufsize",         sizeof(inbuf_t));
    fprintf(chan,STAT_FORMAT"\n", "inbufmalloc",       stat_inbufmalloc);
    fprintf(chan,STAT_FORMAT"\n", "inbufmallocsize",   INBUFALLOCNUM*sizeof(inbuf_t));
    fprintf(chan,STAT_FORMAT"\n", "inbufmalloctotal",  stat_inbufmalloc*INBUFALLOCNUM*sizeof(inbuf_t));
    fprintf(chan,STAT_FORMAT"\n", "inbufmax",          stat_inbufmax);
    fprintf(chan,STAT_FORMAT"\n", "inbufnow",          stat_inbufnow);
    fprintf(chan,"\n");
    fprintf(chan,STAT_FORMAT"\n", "elemsize",          size_elem_t);
    fprintf(chan,STAT_FORMAT"\n", "elemmalloc",        stat_elemmalloc);
    fprintf(chan,STAT_FORMAT"\n", "elemmallocsize",    LISTALLOCNUM*size_elem_t);
    fprintf(chan,STAT_FORMAT"\n", "elemmalloctotal",   stat_elemmalloc*LISTALLOCNUM*size_elem_t);
    fprintf(chan,STAT_FORMAT"\n", "elemmax",           stat_elemmax);
    fprintf(chan,STAT_FORMAT"\n", "elemnow",           stat_elemnow);
    fprintf(chan,"]\n");
}