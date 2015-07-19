#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "parse.h"
#include "list.h"

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

#define state_t_size FRAG

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
 
static void printg_next(int s, int indent) {
    int *pn, ss, n, nn, i;
    char *s_name, *n_name;

    ss = s & 0xFF;
    s_name = *state_name[ss];
    pn = state_next[ss];
    while (1) {
        n = *pn++;
	nn = n & 0xFF;        
        if (!nn) break;
        n_name = *state_name[nn];
        for (i = indent; i--; ) putc (' ', stdout);
	printf("< %s %s > {\n", s_name, n_name);
	if (n & REC) continue;
        if ((n ^ s) & TWO) continue;
        printg_next(n, indent+2);
        for (i = indent; i--; ) putc (' ', stdout);
	printf("}\n");
    }
}

void printg (void) {
    printg_next(ACT, 0);
}

static int parse_next(int s, unsigned char *inp) {
    int *pn, n, nn, rc;

#if 1
    int ss;
    char *s_name;

    ss = s & 0xFF;
    s_name = *state_name[ss];
    printf("%s\n", s_name);
#endif

    rc = 0;
    while (1) {
        n = *pn++;
        nn = n & 0xFF;
        if (!nn) break;  // FIXME
    
//        if (n & REC) continue;     // FIXME - maybe allocate new inp buffers?

        if ((n ^ s) & TWO) continue;

	if (n & REP) { while (( rc = parse_next(n, inp) ) == 0); break; }
	if (n & ALT) if (( rc = parse_next(n, inp) ) != 0) continue;
	if (n & OPT) if (( rc = parse_next(n, inp) ) == 0) break;
        rc = parse_next(n, inp);
    }
    return rc;
}

int parse(unsigned char *inp) {
    return parse_next(ACT, inp);
}

#if 0
typedef struct {
    elem_t
	fraglist,       // string fragments forming a single string
	npathlist,      // strings components forming a path
	nlistlist,      // paths descibing sets of nodes or endpoints
        edgelist,       // list of endpoints sets describing one or more edges
        elistlist,      // list of edges or of edge sets
    	nproplist,      // list of node properties
    	eproplist,      // list of edge properties
	cproplist,      // list of container properties
        containerlist;  // list of acts
} act_t;

static int opencloseblock(act_t *act, state_t state) {
    return 0;
}

int parse(unsigned char *inp) {
    unsigned char c;
    int rc, linecharcnt, linecnt, charsize;
    int state;
    act_t *act;
    elem_t *fraglist, *npathlist, *nlistlist /*, *edgelist, *elistlist,
	*nproplist, *eproplist, *cproplist, *containerlist */;
    elem_t *frag, *npathelem, *nlistelem;

    act = calloc(sizeof(act_t), 1);

    fraglist = &(act->fraglist);
    npathlist = &(act->npathlist);
    nlistlist = &(act->nlistlist);
/*  edgelist = &(act->edgelist);
    elistlist = &(act->elistlist);
    nproplist = &(act->nproplist);
    eproplist = &(act->eproplist);
    cproplist = &(act->cproplist);
    containerlist = &(act->containerlist); */

    frag = NULL;
    linecharcnt = 0;
    linecnt = 0;
    while ((c = *inp++)) {
        linecharcnt++;
        state = char2state[c];
        charsize = 1;
        if (state & TWO) {
            if ((c = *inp)) {
	        state = char2state[c];

                // we work out what the character pair is by combining the 
                // charpros from both chars  (this means that CRLF is only a single EOL)
                // and then masking out the TWO and other unrelated bits
                state |= charTWOstate2props[state];
                state &= proptypemask;
                if (state) {
		    inp++;
                    charsize++;
	            if ((state & EOL)) {
                        linecnt++;
                        linecharcnt = 0;
                    }
		}
	    }
        }
        if (frag) {    // a frag already exists from a previous iteration of the while loop
            assert (frag->type == STR);
            if (state & frag->state) {  // matching state, just continue appending to frag
                frag->u.str.len += charsize;
            }
	    else {
                if (state & fraglist->state) {  // if matches fraglist 
 		    if (state & (STRING|SPACE)) {  // STRING or SPACE char that can be simply appended
                        frag->u.str.len += charsize;
                    }
                    else if (state & ESCAPE) {  // STRING frag that starts at this char - needs a new frag to skip BSL char
			frag = newelem(STRING, inp-1, 1, 0);
			appendlist(fraglist, frag);
                    }
                    else { // DQTSTR|SQTSTR -- prop that start at next char - so new frag to skip this char
			frag = newelem(state & proptypemask, inp, 0, 0);
			appendlist(fraglist, frag);
                    }
                }
 		else { // new state don't match, so something got terminated and needs promoting
//printj(fraglist);
                    if (fraglist->state & (STRING|ESCAPE)) {
			npathelem = list2elem(fraglist);
			appendlist(npathlist, npathelem);
//printj(npathlist);

//FIXME - something in here to deal with path separators:  :/ <abc>/ <abc> :: <abc> : <abc>
			nlistelem = list2elem(npathlist);
			appendlist(nlistlist, nlistelem);
printj(nlistlist);
		    } 
                    freelist(fraglist);  // free anything that hasn't been promoted,  e.g. WS
                    frag = NULL;
		}
            }
        }

        if (state & (OPEN|CLOSE)) {
	    rc = opencloseblock(act, state);
	    if (rc) {
		fprintf(stderr, "parser error at: %d:%d \"%c\"\n", linecnt,  linecharcnt, c);
                return rc;
            }
        }
// FIXME - deal with OPEN and CLOSE
//     NPATH | NLIST --> EDGE
//     EDGE --> ELIST

        if (!frag) { // no current frag
            if (state & (STRING|ESCAPE)) {
		fraglist->state = (STRING|ESCAPE);
		frag = newelem(STRING, inp-1, 1, 0);
		appendlist(fraglist, frag);
            }
            else if (state & (DQTSTR|SQTSTR)) {
		fraglist->state = (STRING|ESCAPE);
		frag = newelem(STRING, inp, 0, 0);
		appendlist(fraglist, frag);
            }
	    else if (state & SPACE) {
		fraglist->state = SPACE;
		frag = newelem(SPACE, inp-1, 1, 0);
		appendlist(fraglist, frag);
	    }
// FIXME - something for quoted strings
	}
    }
    return 0;
}
#endif
