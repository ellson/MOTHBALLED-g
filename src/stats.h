extern long stat_filecount;
extern long stat_inchars;
extern long stat_actcount;
extern long stat_containercount;
extern long stat_inbufmalloc;
extern long stat_inbufmax;
extern long stat_inbufnow;
extern long stat_elemmalloc;
extern long stat_elemmax;
extern long stat_elemnow;

void print_stats(FILE *chan, struct timespec *starttime);
