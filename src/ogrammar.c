#include "grammar.h"

#define NEXT(s,...) char *s##_str=#s;int s##_nxt[]={__VA_ARGS__, NULL}

NEXT(NUL,	    ONE );
NEXT(ACT,	    ACTION|OPT, SUBJECT, ATTRIBUTES|OPT, CONTAINER|OPT, TERM|OPT );
NEXT(ACTION,	    TLD );
NEXT(SUBJECT,	    OBJECT|ALT, OBJECT_LIST|ALT );
NEXT(ATTRIBUTES,    LBT, ATTR, ATTR_MORE|REP|OPT, RBT );
NEXT(ATTR_MORE,	    SPACE, ATTR );
NEXT(CONTAINER,	    LBE, ATTRIBUTES|OPT, CONTENTS|OPT, RBE );
NEXT(CONTENTS,      ACT|REC, CONTENTS_MORE|REP|OPT );
NEXT(CONTENTS_MORE, SPACE, ACT|REC );
NEXT(OBJECT_LIST,   LPN, OBJECT, OBJECT_MORE|REP|OPT, RPN );
NEXT(OBJECT_MORE,   SPACE, OBJECT );
NEXT(OBJECT,	    EDGE|ALT, NODE|ALT );
NEXT(EDGE,	    LAN, TAIL, HEAD|REP, RAN, DISAMBIGUATOR|OPT );
NEXT(TAIL,	    ENDPOINT|ALT, ENDPOINT_SET|ALT );
NEXT(HEAD,	    ENDPOINT|ALT, ENDPOINT_SET|ALT );
NEXT(ENDPOINT_SET,  LPN, ENDPOINT, ENDPOINT_MORE|REP, RPN );
NEXT(ENDPOINT_MORE, SPACE, ENDPOINT );
NEXT(ENDPOINT,      ANCESTOR_MEMBER|ALT, FAMILY_MEMBER|ALT, PORT|ALT);
NEXT(ANCESTOR_MEMBER, HAT, ANCESTOR_MORE|REP|OPT, DESCENDENT|REP, PORT|OPT );
NEXT(ANCESTOR_MORE, FSL, CLN );
NEXT(FAMILY_MEMBER, NODE, DESCENDENT|REP|OPT, PORT|OPT );
NEXT(DESCENDENT,    FSL, NODE );
NEXT(PORT,	    CLN, PORTID );
NEXT(NODE,	    NODEID, DISAMBIGUATOR|OPT );
NEXT(DISAMBIGUATOR, TIC, DISAMBID );
NEXT(ATTR,	    ATTRID, VALASSIGN|OPT );
NEXT(VALASSIGN,	    EQL, VALUE );
NEXT(NODEID,	    STRING|ALT, AST|ALT );
NEXT(DISAMBID,	    STRING|ALT, AST|ALT );
NEXT(PORTID,	    STRING|ALT, AST|ALT );
NEXT(ATTRID,	    STRING );
NEXT(VALUE,	    STRING );
NEXT(STRING,	    FRAG, FRAG|REP|OPT );
NEXT(FRAG,	    ABC|ALT, DQFRAG|ALT, SQFRAG|ALT, BSL|TWO|ESCAPE|ALT );
NEXT(DQFRAG,        DQT, ABC|REP|OPT, DQT );
NEXT(SQFRAG,        SQT, ABC|REP|OPT, SQT );
NEXT(SPACE,         WS );
NEXT(TERM,	    SCN|OPT );
NEXT(ABC,	    ONE );
NEXT(WS,	    ONE );
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
NEXT(HAT,	    ONE );
NEXT(TIC,	    ONE );
NEXT(TLD,	    ONE );
NEXT(LF,	    CR|TWO|EOL|ALT, ONE|EOL|ALT );
NEXT(CR,	    LF|TWO|EOL|ALT, ONE|EOL|ALT );

static int *state_next[] = {
    NUL_nxt,
    ACT_nxt,
    ACTION_nxt,
    SUBJECT_nxt,
    ATTRIBUTES_nxt,
    ATTR_MORE_nxt,
    CONTAINER_nxt,
    CONTENTS_nxt,
    CONTENTS_MORE_nxt,
    OBJECT_LIST_nxt,
    OBJECT_MORE_nxt,
    OBJECT_nxt,
    EDGE_nxt,
    TAIL_nxt,
    HEAD_nxt,
    ENDPOINT_SET_nxt,
    ENDPOINT_MORE_nxt,
    ENDPOINT_nxt,
    ANCESTOR_MEMBER_nxt,
    ANCESTOR_MORE_nxt,
    FAMILY_MEMBER_nxt,
    DESCENDENT_nxt,
    PORT_nxt,
    NODE_nxt,
    DISAMBIGUATOR_nxt,
    ATTR_nxt,
    VALASSIGN_nxt,
    NODEID_nxt,
    DISAMBID_nxt,
    PORTID_nxt,
    ATTRID_nxt,
    VALUE_nxt,
    STRING_nxt,
    FRAG_nxt,
    DQFRAG_nxt,
    SQFRAG_nxt,
    SPACE_nxt,
    TERM_nxt,
    ABC_nxt,
     WS_nxt,
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
    HAT_nxt,
    TIC_nxt,
    TLD_nxt,
     LF_nxt,
     CR_nxt
};

static char **state_name[] = {
    &NUL_str,
    &ACT_str,
    &ACTION_str,
    &SUBJECT_str,
    &ATTRIBUTES_str,
    &ATTR_MORE_str,
    &CONTAINER_str,
    &CONTENTS_str,
    &CONTENTS_MORE_str,
    &OBJECT_LIST_str,
    &OBJECT_MORE_str,
    &OBJECT_str,
    &EDGE_str,
    &TAIL_str,
    &HEAD_str,
    &ENDPOINT_SET_str,
    &ENDPOINT_MORE_str,
    &ENDPOINT_str,
    &ANCESTOR_MEMBER_str,
    &ANCESTOR_MORE_str,
    &FAMILY_MEMBER_str,
    &DESCENDENT_str,
    &PORT_str,
    &NODE_str,
    &DISAMBIGUATOR_str,
    &ATTR_str,
    &VALASSIGN_str,
    &NODEID_str,
    &DISAMBID_str,
    &PORTID_str,
    &ATTRID_str,
    &VALUE_str,
    &STRING_str,
    &FRAG_str,
    &DQFRAG_str,
    &SQFRAG_str,
    &SPACE_str,
    &TERM_str,
    &ABC_str,
     &WS_str,
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
    &HAT_str,
    &TIC_str,
    &TLD_str,
     &LF_str,
     &CR_str,
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
    ABC,ABC,ABC,LBT,BSL,RBT,HAT,ABC, /*  X   Y   Z   [   \   ]   ^   _  */
    TIC,ABC,ABC,ABC,ABC,ABC,ABC,ABC, /*  `   a   b   c   d   e   f   g  */
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
