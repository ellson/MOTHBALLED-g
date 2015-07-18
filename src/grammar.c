/*
act		SEQ: action? subject properties? container? term?

subject		ALT: obj objlist
properties	SEQ: LBR attr_val* RBR
container       SEQ: LBE properties? act* RBE

objlist		SEQ: LPA obj* RPA     // list must be all same obj type
obj		ALT: edge node

edge		SEQ: LAN leg leg leg* RAN disambiguator?

leg		ALT: endpoint enpointset
endpointset	SEQ: LPA endpoint* RPA

endpoint        SEQ: ancestor* descendent* node port?
descendent      SEQ: nodeid FSL
port		SEQ: CLN portid

nodelist	SEQ: LPA node* RPA
node		SEQ: nodeid disambiguator?

disabiguator	SEQ: disamintro disamid

attr_val	SEQ: attrid valueassign?
valueassign	SEQ: EQL value



nodeid		ALT: string pattern 
disamid		ALT: string pattern 
portid		ALT: string pattern

value		SEQ: string
attrid		SEQ: string

string		SEQ: frag frag*

action		TOK: '~'
term		TOK: ';'
LBE             TOK: '{'
RBE             TOK: '}'
LPA             TOK: '('
RPA             TOK: ')'
LBR             TOK: '['
RBR             TOK: ']'
LAN             TOK: '<'
RAN             TOK: '>'
EQL             TOK: '='
FSL             TOK: '/'
pattern		TOK: '*'

ancestor	TWO: ':/'
disamintro	TWO: '::'
*/


// missing:  quoted strings, escapes, comments


typedef enum {
    // input charclass
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
    // grammar states
    ACT,
    ACTION,
    SUBJECT,
    PROPERTIES,
    CONTAINER,
    OBJLIST,
    OBJ,
    EDGE,
    LEG,
    ENDPOINTSET,
    ENDPOINT,
    DESCENDENT,
    PORT,
    NODELIST,
    NODE,
    DISAMBIGUATOR,
    ATTR_VAL,
    VALUEASSIGN,
    NODEID,
    DISAMID,
    PORTID,
    VALUE 
} charclass_t;

typedef enum {
   NONE         = 0,
   STRING       = 1<<0,
   TWO          = 1<<1,
   SPACE        = 1<<2,
   OPEN         = 1<<3,
   CLOSE        = 1<<4,
   PARENTHESIS  = 1<<5,
   ANGLEBRACKET = 1<<6,
   BRACKET      = 1<<7,
   BRACE        = 1<<8,
   DQTSTR       = 1<<9,
   SQTSTR       = 1<<10,
   EOL          = 1<<11,
   CMNTBEG      = 1<<12,
   CMNTEND      = 1<<13,
   CMNTEOLBEG   = 1<<14,
   CMNTEOLEND   = 1<<15,
   CMNTSTR      = 1<<16,
   ESCAPE       = 1<<17,
   // grammar props
   SEQ          = 1<<18,
   ALT          = 1<<19,
   OPT          = 1<<20,
   MULTI        = 1<<21,
} charprops_t;

    state[ACT]		= {SEQ,	ACTION|OPT, SUBJECT, PROPERTIES|OPT, CONTAINER|OPT, TERM|OPT, NUL };
    state[ACTION]	= {SEQ, TLD, NUL };
    state[SUBJECT]	= {ALT,	OBJ, OBJLIST, NUL };
    state[PROPERTIES]	= {SEQ,	LBR, ATTR_VAL|MULT, RBR, NUL };
    state[CONTAINER]	= {SEQ,	LBE, PROPERTIES|OPT, ACT|MULT, RBE, NUL };
    state[OBJLIST]	= {SEQ,	LPA, OBJ|MULT, RPA, NUL };
    state[OBJ]		= {ALT,	EDGE, NODE, NUL };
    state[EDGE]		= {SEQ,	LAN, LEG, LEG, LEG|MULT, RAN, DISAMBGUATOR|OPT, NUL };
    state[LEG]		= {ALT,	ENDPOINT, ENDPOINTSET, NUL };
    state[ENDPOINTSET]	= {SEQ,	LPA, ENDPOINT|MULT, RPA, NUL };
    state[ENDPOINT]	= {SEQ,	ANCESTOR|MULT, DESCENDENT|MULT, NODE, PORT|OPT, NUL };
    state[DESCENDENT]	= {SEQ,	NODEID, FSL, NUL };
    state[PORT]		= {SEQ,	CLN, PORTID, NUL };
    state[NODELIST]	= {SEQ,	LPA, NODE|MULT, rpa, NUL };
    state[NODE]		= {SEQ,	NODEID, DISAMBIGUATOR|OPT, NUL };
    state[DISAMBIGUATOR]= {SEQ,	DISAMBINTRO, DISAMBID, NUL };
    state[ATTR_VAL]	= {SEQ,	ATTRID VALUEASSIGN|OPT, NUL };
    state[VALUEASSIGN]	= {SEQ,	EQL, VALUE, NUL };
    state[NODEID]	= {ALT,	STRING, AST, NUL };
    state[DISAMID]	= {ALT,	STRING, AST, NUL };
    state[PORTID]	= {ALT,	STRING, AST, NUL };
    state[VALUE]	= {SEQ,	STRING, NUL }}
