// every input character is a member of two possible states:  ONE and TWO
typedef enum {
    // grammar states
    NUL			= 0,	// EOF
    ACT			= 2,	//start
    ACTION		= 4,
    SUBJECT		= 6,
    ATTRIBUTES		= 8,
    ATTR_MORE		= 10,
    CONTAINER		= 12,
    CONTENTS		= 14,
    CONTENTS_MORE	= 16,
    OBJECT_LIST		= 18,
    OBJECT_MORE		= 20,
    OBJECT		= 22,
    EDGE		= 24,
    TAIL		= 26,
    HEAD		= 28,
    ENDPOINT_SET	= 30,
    ENDPOINT_MORE	= 32,
    ENDPOINT		= 34,
    ANCESTOR_MEMBER     = 36,
    ANCESTOR_MORE	= 38,
    FAMILY_MEMBER	= 40,
    DESCENDENT		= 42,
    PORT		= 44,
    NODE		= 46,
    DISAMBIGUATOR	= 48,
    ATTR		= 50,
    VALASSIGN		= 52,
    NODEID		= 54,
    DISAMBID		= 56,
    PORTID		= 58,
    ATTRID		= 60,
    VALUE		= 62,
    STRING		= 64,
    FRAG		= 66,
    DQFRAG		= 68,
    SQFRAG		= 70,
    SPACE		= 72,
    TERM		= 74,
    // also input states
    ABC			= 76,	// simple_string_character
     WS			= 78,	// whitespace
    DQT			= 80,	// '"'
    SQT			= 82,	// '''
    LPN			= 84,	// '('
    RPN			= 86,	// ')'
    LAN			= 88,	// '<'
    RAN			= 90,	// '>'
    LBT			= 92,	// '['
    RBT			= 94,	// ']'
    LBE			= 96,	// '{'
    RBE			= 98,	// '}'
    FSL			= 100,	// '/'
    BSL			= 102,	// '\'
    OCT			= 104,	// '#'
    AST			= 106,	// '*'
    CLN			= 108,	// ':'
    SCN			= 110,	// ';'
    EQL			= 112,	// '='
    HAT			= 114,	// '^'
    TIC			= 116,	// '`'
    TLD			= 118,	// '~'
     LF			= 120,	// lineend
     CR			= 122	// return

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
