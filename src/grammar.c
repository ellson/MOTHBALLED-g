#include "grammar.h"

#define NEXT(s,...) char *s##_str=#s;int s##_nxt[]={__VA_ARGS__, NULL}

NEXT(NUL,	    ONE );
NEXT(ABC,	    ONE );
NEXT(WS,	    ONE );
NEXT(LF,	    CR|TWO|EOL|ALT, ONE|EOL|ALT );
NEXT(CR,	    LF|TWO|EOL|ALT, ONE|EOL|ALT );
NEXT(DQT,	    ONE );
NEXT(SQT,	    ONE );
NEXT(LPN,	    ONE );
NEXT(RPN,	    ONE );
NEXT(LAN,	    ONE );
NEXT(RAN,	    ONE );
NEXT(LBR,	    ONE );
NEXT(RBR,	    ONE );
NEXT(LBE,	    ONE );
NEXT(RBE,	    ONE );
NEXT(FSL,	    ONE|ALT, FSL|TWO|COMMENTEOL|ALT, AST|TWO|COMMENTBEG|ALT );
NEXT(BSL,	    BSL|TWO|ESCAPE|ALT, DQT|TWO|ESCAPE|ALT, SQT|TWO|ESCAPE|ALT, ONE|ALT );
NEXT(OCT,	    COMMENTBEG );
NEXT(AST,	    ONE|ALT, FSL|TWO|COMMENTEND|ALT );
NEXT(CLN,	    CLN|TWO|DISAMBIG|ALT, FSL|TWO|PARENT|ALT, ONE|ALT );
NEXT(SCN,	    ONE );
NEXT(EQL,	    ONE );
NEXT(TLD,	    ONE );
NEXT(ACT,	    ACTION|OPT, SUBJECT, PROPERTIES|OPT, CONTAINER|OPT, TERM|OPT );
NEXT(ACTION,	    TLD );
NEXT(SUBJECT,	    OBJECT|ALT, OBJECT_LIST|ALT );
NEXT(PROPERTIES,    LBT, ATTRIBUTE|REP|OPT, RBT );
NEXT(CONTAINER,	    LBE, PROPERTIES|OPT, ACT|REC|REP|OPT, RBE );
NEXT(TERM,	    SCN|OPT );
NEXT(OBJECT_LIST,   LPN, OBJECT|REP, RPN );
NEXT(OBJECT,	    EDGE|ALT, NODE|ALT );
NEXT(EDGE,	    LAN, TAIL, HEAD|REP, RAN, DISAMBIGUATOR|OPT );
NEXT(TAIL,	    ENDPOINT|ALT, ENDPOINT_SET|ALT );
NEXT(HEAD,	    ENDPOINT|ALT, ENDPOINT_SET|ALT );
NEXT(ENDPOINT_SET,  LPN, ENDPOINT|REP, RPN );
NEXT(ENDPOINT,	    ANCESTOR|REP|OPT, DESCENDENT|REP|OPT, NODE, PORT|OPT );
NEXT(DESCENDENT,    NODEID, FSL );
NEXT(PORT,	    CLN, PORTID );
NEXT(NODE,	    NODEID, DISAMBIGUATOR|OPT );
NEXT(DISAMBIGUATOR, DISAMBINTRO, DISAMBID );
NEXT(ATTRIBUTE,	    ATTRID, VALASSIGN|OPT );
NEXT(VALASSIGN,	    EQL, VALUE );
NEXT(NODEID,	    STRING|ALT, AST|ALT );
NEXT(DISAMBID,	    STRING|ALT, AST|ALT );
NEXT(PORTID,	    STRING|ALT, AST|ALT );
NEXT(ATTRID,	    STRING );
NEXT(VALUE,	    STRING );
NEXT(DISAMBINTRO,   CLN|TWO|DISAMBIG );
NEXT(ANCESTOR,	    CLN|TWO|PARENT );
NEXT(STRING,	    DQSTR|ALT, SQSTR|ALT, ABC|ALT );
NEXT(DQSTR,         DQT, FRAG|REP, DQT );
NEXT(SQSTR,         SQT, FRAG|REP, SQT );
NEXT(FRAG,	    BSL|TWO|ESCAPE|ALT, ABC|ALT );

static int *state_next[] = {
    NUL_nxt,
    ABC_nxt,
    WS_nxt,
    LF_nxt,
    CR_nxt,
    DQT_nxt,
    SQT_nxt,
    LPN_nxt,
    RPN_nxt,
    LAN_nxt,
    RAN_nxt,
    LBR_nxt,
    RBR_nxt,
    LBE_nxt,
    RBE_nxt,
    FSL_nxt,
    BSL_nxt,
    OCT_nxt,
    AST_nxt,
    CLN_nxt,
    SCN_nxt,
    EQL_nxt,
    TLD_nxt,
    ACT_nxt,
    ACTION_nxt,
    SUBJECT_nxt,
    PROPERTIES_nxt,
    CONTAINER_nxt,
    TERM_nxt,
    OBJECT_LIST_nxt,
    OBJECT_nxt,
    EDGE_nxt,
    TAIL_nxt,
    HEAD_nxt,
    ENDPOINT_SET_nxt,
    ENDPOINT_nxt,
    DESCENDENT_nxt,
    PORT_nxt,
    NODE_nxt,
    DISAMBIGUATOR_nxt,
    ATTRIBUTE_nxt,
    VALASSIGN_nxt,
    NODEID_nxt,
    DISAMBID_nxt,
    PORTID_nxt,
    ATTRID_nxt,
    VALUE_nxt,
    DISAMBINTRO_nxt,
    ANCESTOR_nxt,
    STRING_nxt,
    DQSTR_nxt,
    SQSTR_nxt,
    FRAG_nxt
};

static char **state_name[] = {
    &NUL_str,
    &ABC_str,
    &WS_str,
    &LF_str,
    &CR_str,
    &DQT_str,
    &SQT_str,
    &LPN_str,
    &RPN_str,
    &LAN_str,
    &RAN_str,
    &LBR_str,
    &RBR_str,
    &LBE_str,
    &RBE_str,
    &FSL_str,
    &BSL_str,
    &OCT_str,
    &AST_str,
    &CLN_str,
    &SCN_str,
    &EQL_str,
    &TLD_str,
    &ACT_str,
    &ACTION_str,
    &SUBJECT_str,
    &PROPERTIES_str,
    &CONTAINER_str,
    &TERM_str,
    &OBJECT_LIST_str,
    &OBJECT_str,
    &EDGE_str,
    &TAIL_str,
    &HEAD_str,
    &ENDPOINT_SET_str,
    &ENDPOINT_str,
    &DESCENDENT_str,
    &PORT_str,
    &NODE_str,
    &DISAMBIGUATOR_str,
    &ATTRIBUTE_str,
    &VALASSIGN_str,
    &NODEID_str,
    &DISAMBID_str,
    &PORTID_str,
    &ATTRID_str,
    &VALUE_str,
    &DISAMBINTRO_str,
    &ANCESTOR_str,
    &STRING_str,
    &DQSTR_str,
    &SQSTR_str,
    &FRAG_str
};

// Every character is grouped into exactly one state_t
// All the characters in a state_t are treated identially by the parser
//
// state_t defaults to int, which would make this table 768 bytes larger
unsigned char char2state[] = {
    NUL,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /* NUL SOH STX ETX EOT ENQ ACK BEL */
    ABC, WS, LF,ABC,ABC, CR,ABC,ABC, /*  BS TAB  LF  VT  FF  CR  SO  SI */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /* DLE DC1 DC2 DC3 DC4 NAK STN ETB */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /* CAN  EM SUB ESC  FS  GS  RS  US */
     WS,ABC,DQT,OCT,ABC,ABC,ABC,SQT, /* SPC  !   "   #   $   %   &   '  */
    LPN,RPN,AST,ABC,ABC,ABC,ABC,FSL, /*  (   )   *   +   ,  -   .   /  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  0   1   2   3   4   5   6   7  */
    ABC,ABC,CLN,SCN,LAN,EQL,RAN,ABC, /*  8   9   :   ;   <   =   >   ?  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  @   A   B   C   D   E   F   G  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  H   I   J   K   L   M   N   O  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  P   Q   R   S   T   U   V   W  */
    ABC,ABC,ABC,LBT,BSL,RBT,ABC,ABC, /*  X   Y   Z   [   \   ]   ^   _  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  `   a   b   c   d   e   f   g  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  h   i   j   k   l   m   n   o  */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  p   q   r   s   t   u   v   w  */
    ABC,ABC,ABC,LBE,ABC,RBE,TLD,ABC, /*  x   y   z   {   |   }   ~  DEL */
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
    ABC,ABC,ABC,ABC,ABC,ABC,ABC,ABC,
};
