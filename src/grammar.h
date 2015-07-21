typedef enum {
    ALT          = 1<<15,   // alternive - one must be satisfied
    OPT          = 1<<14,   // optional
    REP          = 1<<13,   // repeatable   ( REP|OPT means 0 or more )
    SREP         = 1<<12,   // repeatable with <SPACE> separation   ( SREP|OPT means 0 or more )
    REC          = 1<<11   // recursion
} props_t;
