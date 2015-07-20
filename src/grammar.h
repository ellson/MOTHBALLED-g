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
    TLD = 44,       // '~'

    ACT			= 46,
    ACTION		= 48,
    SUBJECT		= 50,
    ATTRIBUTES		= 52,
    ATTR_MORE		= 54,
    CONTAINER		= 56,
    CONTENTS		= 58,
    CONTENTS_MORE       = 60,
    TERM		= 62,
    OBJECT_LIST		= 64,
    OBJECT_MORE		= 66,
    OBJECT		= 68,
    EDGE		= 70,
    TAIL		= 72,
    HEAD		= 74,
    ENDPOINT_SET	= 76,
    ENDPOINT_MORE	= 78,
    ENDPOINT		= 80,
    DESCENDENT		= 82,
    PORT		= 84,
    NODE		= 86,
    DISAMBIGUATOR	= 88,
    ATTR		= 90,
    VALASSIGN		= 92,
    NODEID		= 94,
    DISAMBID		= 96,
    PORTID		= 98,
    ATTRID		= 100,
    VALUE		= 102,
    DISAMBINTRO		= 104,
    ANCESTOR		= 106,
    STRING		= 108,
    FRAG		= 110,
    DQFRAG		= 112,
    SQFRAG		= 114,
    SPACE		= 116
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
