typedef enum {
    // grammar states
    NUL,       // EOF
    ABC,       // simple_string_character
     WS,       // whitespace
     LF,       // newline
     CR,       // return
    DQT,       // '"'
    SQT,       // '''
    LPN,       // '('
    RPN,       // ')'
    LAN,       // '<'
    RAN,       // '>'
    LBT,       // '['
    RBT,       // ']'
    LBE,       // '{'
    RBE,       // '}'
    FSL,       // '/'
    BSL,       // '\'
    OCT,       // '#'
    AST,       // '*'
    CLN,       // ':'
    SCN,       // ';'
    EQL,       // '='
    TLD,       // '~'
    ACT,
    ACTION,
    SUBJECT,
    PROPERTIES,
    CONTAINER,
    TERM,
    OBJECT_LIST,
    OBJECT,
    EDGE,
    TAIL,
    HEAD,
    ENDPOINT_SET,
    ENDPOINT,
    DESCENDENT,
    PORT,
    NODE,
    DISAMBIGUATOR,
    ATTRIBUTE,
    VALASSIGN,
    NODEID,
    DISAMBID,
    PORTID,
    ATTRID,
    VALUE,
    DISAMBINTRO,
    ANCESTOR,
    STRING,
    DQSTR,
    SQSTR,
    FRAG
} state_t;

typedef enum {
    ONE          = 0,       // because __VS_ARGS__ doesn't like to be empty
    TWO          = 1<<8,    // a two character sequence
    EOL          = 1<<9,    // CRLF or LFCR or LF or CR
    COMMENTBEG   = 1<<10,   // /*
    COMMENTEND   = 1<<11,   // */
    COMMENTEOL   = 1<<12,   // #  or //
    DISAMBIG     = 1<<13,   // ::
    PARENT       = 1<<14,   // :/
    ESCAPE       = 1<<15,   // \x
    ALT          = 1<<16,   // alternive - one must be satisfied
    OPT          = 1<<17,   // optional
    REP          = 1<<18,   // repeatable   ( REP|OPT means 0 or morea )
    REC          = 1<<19,   // recursion
} props_t;
