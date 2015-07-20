// every input character is a member of two possible states:  ONE and TWO
typedef enum {
    // grammar states
    NUL =  0,       // EOF
    ABC =  2,       // simple_string_character
     WS =  4,       // whitespace
     LF =  6,       // newline
     CR =  8,       // return
    DQT = 10,       // '"'
    SQT = 12,       // '''
    LPN = 14,       // '('
    RPN = 16,       // ')'
    LAN = 18,       // '<'
    RAN = 20,       // '>'
    LBT = 22,       // '['
    RBT = 24,       // ']'
    LBE = 26,       // '{'
    RBE = 28,       // '}'
    FSL = 30,       // '/'
    BSL = 32,       // '\'
    OCT = 34,       // '#'
    AST = 36,       // '*'
    CLN = 30,       // ':'
    SCN = 40,       // ';'
    EQL = 42,       // '='
    HAT = 44,       // '^'
    TIC = 46,       // '`'
    TLD = 48,       // '~'

    ACT			= 50,
    ACTION		= 52,
    SUBJECT		= 54,
    ATTRIBUTES		= 56,
    ATTR_MORE		= 58,
    CONTAINER		= 60,
    CONTENTS		= 62,
    CONTENTS_MORE       = 64,
    TERM		= 66,
    OBJECT_LIST		= 68,
    OBJECT_MORE		= 70,
    OBJECT		= 72,
    EDGE		= 74,
    TAIL		= 76,
    HEAD		= 78,
    ENDPOINT_SET	= 80,
    ENDPOINT_MORE	= 82,
    ENDPOINT		= 84,
    ANCESTOR_MEMBER     = 86,
    ANCESTOR_MORE       = 88,
    FAMILY_MEMBER       = 90,
    DESCENDENT		= 92,
    PORT		= 94,
    NODE		= 96,
    DISAMBIGUATOR	= 98,
    ATTR		= 100,
    VALASSIGN		= 102,
    NODEID		= 104,
    DISAMBID		= 106,
    PORTID		= 108,
    ATTRID		= 110,
    VALUE		= 112,
    STRING		= 114,
    FRAG		= 116,
    DQFRAG		= 118,
    SQFRAG		= 120,
    SPACE		= 122
} state_t;

typedef enum {
    ONE          = 0,       // because __VS_ARGS__ doesn't like to be empty
    TWO          = 1,       // a two character sequence
    EOL          = 1<<8,    // CRLF or LFCR or LF or CR
    COMMENTBEG   = 1<<9,    // /*
    COMMENTEND   = 1<<10,   // */
    COMMENTEOL   = 1<<11,   // #  or //
    DISAMBIG     = 1<<12,   // ::
    PARENT       = 1<<13,   // :/
    ESCAPE       = 1<<14,   // \x
    ALT          = 1<<15,   // alternive - one must be satisfied
    OPT          = 1<<16,   // optional
    REP          = 1<<17,   // repeatable   ( REP|OPT means 0 or morea )
    REC          = 1<<18,   // recursion
} props_t;
