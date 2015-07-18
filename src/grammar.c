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
    // grammar states
    ACT,
    ACTION,
    SUBJECT,
    PROPERTIES,
    CONTAINER,
    TERM,
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
    DISAMB,
    ATTR_VAL,
    VALASSIGN,
    NODEID,
    DISAMBID,
    PORTID,
    ATTRID,
    VALUE,
    DISAMBINTRO,
    ANCESTOR,
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
   ALT          = 1<<18,   // alternive - one must be satisfied
   OPT          = 1<<19,   // optional
   REP          = 1<<20,   // repeatable   ( REP|OPT means 0 or morea )
} charprops_t;

    int ACT_nxt[] =		{ACTION|OPT, SUBJECT, PROPERTIES|OPT, CONTAINER|OPT, TERM|OPT, NUL};
    int ACTION_nxt[] =		{TLD, NUL};
    int SUBJECT_nxt[] =		{OBJ|ALT, OBJLIST|ALT, NUL};
    int PROPERTIES_nxt[] =	{LBT, ATTR_VAL|REP|OPT, RBT, NUL};
    int CONTAINER_nxt[] =	{LBE, PROPERTIES|OPT, ACT|REP|OPT, RBE, NUL};
    int TERM_nxt[] =            {SCN|OPT, NUL};
    int OBJLIST_nxt[] =		{LPN, OBJ|REP|OPT, RPN, NUL};
    int OBJ_nxt[] =		{EDGE|ALT, NODE|ALT, NUL};
    int EDGE_nxt[] =		{LAN, LEG, LEG, LEG|REP|OPT, RAN, DISAMB|OPT, NUL};
    int LEG_nxt[] =		{ENDPOINT|ALT, ENDPOINTSET|ALT, NUL};
    int ENDPOINTSET_nxt[] =	{LPN, ENDPOINT|REP|OPT, RPN, NUL};
    int ENDPOINT_nxt[] =	{ANCESTOR|REP|OPT, DESCENDENT|REP|OPT, NODE, PORT|OPT, NUL};
    int DESCENDENT_nxt[] =	{NODEID, FSL, NUL};
    int PORT_nxt[] =		{CLN, PORTID, NUL};
    int NODELIST_nxt[] =	{LPN, NODE|REP|OPT, RPN, NUL};
    int NODE_nxt[] =		{NODEID, DISAMB|OPT, NUL};
    int DISAMB_nxt[] =		{DISAMBINTRO, DISAMBID, NUL};
    int ATTR_VAL_nxt[] =	{ATTRID, VALASSIGN|OPT, NUL};
    int VALASSIGN_nxt[] =	{EQL, VALUE, NUL};
    int NODEID_nxt[] =		{STRING|ALT, AST|ALT, NUL};
    int DISAMBID_nxt[] =	{STRING|ALT, AST|ALT, NUL};
    int PORTID_nxt[] =		{STRING|ALT, AST|ALT, NUL};
    int ATTRID_nxt[] =		{STRING, NUL};
    int VALUE_nxt[] =		{STRING, NUL};
    int DISAMBINTRO_nxt[] =	{CLN, CLN, NUL};
    int ANCESTOR_nxt[] =	{CLN, FSL, NUL};

int *next[] = {
    ACT_nxt,
    ACTION_nxt,
    SUBJECT_nxt,
    PROPERTIES_nxt,
    CONTAINER_nxt,
    TERM_nxt,
    OBJLIST_nxt,
    OBJ_nxt,
    EDGE_nxt,
    LEG_nxt,
    ENDPOINTSET_nxt,
    ENDPOINT_nxt,
    DESCENDENT_nxt,
    PORT_nxt,
    NODELIST_nxt,
    NODE_nxt,
    DISAMB_nxt,
    ATTR_VAL_nxt,
    VALASSIGN_nxt,
    NODEID_nxt,
    DISAMBID_nxt,
    PORTID_nxt,
    ATTRID_nxt,
    VALUE_nxt,
    DISAMBINTRO_nxt,
    ANCESTOR_nxt
};

