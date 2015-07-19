#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "parse.h"
#include "list.h"

typedef enum {
    // input state
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
    ATTR_VAL,
    VALASSIGN,
    NODEID,
    DISAMBID,
    PORTID,
    ATTRID,
    VALUE,
    DISAMBINTRO,
    ANCESTOR,
} state_t;

#define state_t_size TLD

typedef enum {
    NONE         = 0,
    STRING       = 1<<8,
    TWO          = 1<<9,
    SPACE        = 1<<10,
    OPEN         = 1<<11,
    CLOSE        = 1<<12,
    PARENTHESIS  = 1<<13,
    ANGLEBRACKET = 1<<14,
    BRACKET      = 1<<14,
    BRACE        = 1<<16,
    DQTSTR       = 1<<17,
    SQTSTR       = 1<<18,
    EOL          = 1<<19,
    CMNTBEG      = 1<<20,
    CMNTEND      = 1<<21,
    CMNTEOLBEG   = 1<<22,
    CMNTEOLEND   = 1<<23,
    CMNTSTR      = 1<<24,
    ESCAPE       = 1<<25,
    // grammar props
    ALT          = 1<<26,   // alternive - one must be satisfied
    OPT          = 1<<27,   // optional
    REP          = 1<<28,   // repeatable   ( REP|OPT means 0 or morea )
    REC          = 1<<29,   // recursion
} props_t;

#define NEXT(s,n) (char* (s)_str "s";int (s)_nxt[]={(n)})

NEXT(NUL,(NONE, NUL));

static int NUL_nxt[] = {NONE, NUL};
static int ABC_nxt[] = {STRING, NUL};
static int WS_nxt[] = {SPACE, NUL};
static int LF_nxt[] = {EOL|CMNTEOLEND|SPACE, NUL};
static int CR_nxt[] = {TWO|EOL|CMNTEOLEND|SPACE, NUL};
static int DQT_nxt[] = {OPEN|CLOSE, NUL};
static int SQT_nxt[] = {OPEN|CLOSE, NUL};
static int LPN_nxt[] = {OPEN|PARENTHESIS, NUL};
static int RPN_nxt[] = {CLOSE|PARENTHESIS, NUL};
static int LAN_nxt[] = {OPEN|ANGLEBRACKET, NUL};
static int RAN_nxt[] = {OPEN|ANGLEBRACKET, NUL};
static int LBR_nxt[] = {OPEN|BRACKET, NUL};
static int RBR_nxt[] = {OPEN|BRACKET, NUL};
static int LBE_nxt[] = {OPEN|BRACE, NUL};
static int RBE_nxt[] = {OPEN|BRACE, NUL};
static int FSL_nxt[] = {TWO|CMNTBEG|CMNTEOLBEG, NUL};
static int BSL_nxt[] = {TWO|ESCAPE, NUL};
static int OCT_nxt[] = {NONE, NUL};
static int AST_nxt[] = {TWO|CMNTEND, NUL};
static int CLN_nxt[] = {NONE, NUL};
static int SCN_nxt[] = {NONE, NUL};
static int EQL_nxt[] = {NONE, NUL};
static int TLD_nxt[] = {NONE, NUL};

static int ACT_nxt[] = {ACTION|OPT, SUBJECT, PROPERTIES|OPT, CONTAINER|OPT, TERM|OPT, NUL};
static int ACTION_nxt[] = {TLD, NUL};
static int SUBJECT_nxt[] = {OBJECT|ALT, OBJECT_LIST|ALT, NUL};
static int PROPERTIES_nxt[] = {LBT, ATTR_VAL|REP|OPT, RBT, NUL};
static int CONTAINER_nxt[] = {LBE, PROPERTIES|OPT, ACT|REC|REP|OPT, RBE, NUL};
static int TERM_nxt[] = {SCN|OPT, NUL};
static int OBJECT_LIST_nxt[] = {LPN, OBJECT|REP, RPN, NUL};
static int OBJECT_nxt[] = {EDGE|ALT, NODE|ALT, NUL};
static int EDGE_nxt[] = {LAN, TAIL, HEAD|REP, RAN, DISAMBIGUATOR|OPT, NUL};
static int TAIL_nxt[] = {ENDPOINT|ALT, ENDPOINT_SET|ALT, NUL};
static int HEAD_nxt[] = {ENDPOINT|ALT, ENDPOINT_SET|ALT, NUL};
static int ENDPOINT_SET_nxt[] = {LPN, ENDPOINT|REP, RPN, NUL};
static int ENDPOINT_nxt[] = {ANCESTOR|REP|OPT, DESCENDENT|REP|OPT, NODE, PORT|OPT, NUL};
static int DESCENDENT_nxt[] = {NODEID, FSL, NUL};
static int PORT_nxt[] = {CLN, PORTID, NUL};
static int NODE_nxt[] = {NODEID, DISAMBIGUATOR|OPT, NUL};
static int DISAMBIGUATOR_nxt[] = {DISAMBINTRO, DISAMBID, NUL};
static int ATTR_VAL_nxt[] = {ATTRID, VALASSIGN|OPT, NUL};
static int VALASSIGN_nxt[] = {EQL, VALUE, NUL};
static int NODEID_nxt[] = {STRING|ALT, AST|ALT, NUL};
static int DISAMBID_nxt[] = {STRING|ALT, AST|ALT, NUL};
static int PORTID_nxt[] = {STRING|ALT, AST|ALT, NUL};
static int ATTRID_nxt[] = {STRING, NUL};
static int VALUE_nxt[] = {STRING, NUL};
static int DISAMBINTRO_nxt[] = {CLN, CLN, NUL};
static int ANCESTOR_nxt[] = {CLN, FSL, NUL};

static char *NUL_str = "NUL";
static char *ABC_str = "ABC";
static char *WS_str = "WS";
static char *LF_str = "LF";
static char *CR_str = "CR";
static char *DQT_str = "DQT";
static char *SQT_str = "SQT";
static char *LPN_str = "LPN";
static char *RPN_str = "RPN";
static char *LAN_str = "LAN";
static char *RAN_str = "RAN";
static char *LBR_str = "LBR";
static char *RBR_str = "RBR";
static char *LBE_str = "LBE";
static char *RBE_str = "RBE";
static char *FSL_str = "FSL";
static char *BSL_str = "BSL";
static char *OCT_str = "OCT";
static char *AST_str = "AST";
static char *CLN_str = "CLN";
static char *SCN_str = "SCN";
static char *EQL_str = "EQL";
static char *TLD_str = "TLD";

static char *ACT_str = "ACT";
static char *ACTION_str = "ACTION";
static char *SUBJECT_str = "SUBJECT";
static char *PROPERTIES_str = "PROPERTIES";
static char *CONTAINER_str = "CONTAINER";
static char *TERM_str = "TERM";
static char *OBJECT_LIST_str = "OBJECT_LIST";
static char *OBJECT_str = "OBJECT";
static char *EDGE_str = "EDGE";
static char *TAIL_str = "TAIL";
static char *HEAD_str = "HEAD";
static char *ENDPOINT_SET_str = "ENDPOINT_SET";
static char *ENDPOINT_str = "ENDPOINT";
static char *DESCENDENT_str = "DESCENDENT";
static char *PORT_str = "PORT";
static char *NODE_str = "NODE";
static char *DISAMBIGUATOR_str = "DISAMBIGUATOR";
static char *ATTR_VAL_str = "ATTR_VAL";
static char *VALASSIGN_str = "VALASSIGN";
static char *NODEID_str = "NODEID";
static char *DISAMBID_str = "DISAMBID";
static char *PORTID_str = "PORTID";
static char *ATTRID_str = "ATTRID";
static char *VALUE_str = "VALUE";
static char *DISAMBINTRO_str = "DISAMBINTRO";
static char *ANCESTOR_str = "ANCESTOR";


static char **next[] = {
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
 
// some of the property bits define the type of string fragment FIXME - may change as OPEN|CLOSE are dealt with ...
#define proptypemask (STRING | SPACE | ESCAPE | PARENTHESIS | ANGLEBRACKET | BRACKET | BRACE | DQTSTR | SQTSTR | CMNTSTR)

props_t charTWOstate2props[] = {
    /* NUL  */  NONE,
    /* ABC  */  ESCAPE, 
    /*  WS  */  ESCAPE,
    /*  LF  */  ESCAPE | EOL,
    /*  CR  */  ESCAPE | EOL,
    /* DQT  */  ESCAPE,
    /* SQT  */  ESCAPE,
    /* LPN  */  ESCAPE,
    /* RPN  */  ESCAPE,
    /* LAN  */  ESCAPE,
    /* RAN  */  ESCAPE,
    /* LBR  */  ESCAPE,
    /* RBR  */  ESCAPE,
    /* LBE  */  ESCAPE,
    /* RBE  */  ESCAPE,
    /* FSL  */  ESCAPE | CMNTEND | CMNTEOLBEG,
    /* BSL  */  ESCAPE,
    /* OCT  */  ESCAPE,
    /* AST  */  ESCAPE | CMNTBEG,
    /* CLN  */  ESCAPE,
    /* SCN  */  ESCAPE,
    /* EQL  */  ESCAPE,
    /* TLD  */  ESCAPE,
};

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

static void printg_next(int s, int indent) {
    int *pn, n, i;
    char *s_name, *n_name;

    s_name = next[s].name;
    pn = next[s].next;
    while (1) {
        n = *pn++;
	if (n & REC) continue;
	n &= 0xFF;        
        if (!n) break;
        n_name = next[n].name;
        for (i = indent; i--; ) putc (' ', stdout);
#if 0
	printf("< %d %d > {\n", s, n);
#else
	printf("< %s %s > {\n", s_name, n_name);
#endif
        printg_next(n, indent+2);
        for (i = indent; i--; ) putc (' ', stdout);
	printf("}\n");
    }
}

void printg (void) {
    printg_next(ACT, 0);
}

int parse(unsigned char *inp) {
    unsigned char c;
    int rc, linecharcnt, linecnt, charsize;
    state_t state;
    int props;
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
        props = next[state][0];
        charsize = 1;
        if (props & TWO) {
            if ((c = *inp)) {
	        state = char2state[c];

                // we work out what the character pair is by combining the 
                // charpros from both chars  (this means that CRLF is only a single EOL)
                // and then masking out the TWO and other unrelated bits
                props |= charTWOstate2props[state];
                props &= proptypemask;
                if (props) {
		    inp++;
                    charsize++;
	            if ((props & EOL)) {
                        linecnt++;
                        linecharcnt = 0;
                    }
		}
	    }
        }
        if (frag) {    // a frag already exists from a previous iteration of the while loop
            assert (frag->type == STR);
            if (props & frag->props) {  // matching props, just continue appending to frag
                frag->u.str.len += charsize;
            }
	    else {
                if (props & fraglist->props) {  // if matches fraglist 
 		    if (props & (STRING|SPACE)) {  // STRING or SPACE char that can be simply appended
                        frag->u.str.len += charsize;
                    }
                    else if (props & ESCAPE) {  // STRING frag that starts at this char - needs a new frag to skip BSL char
			frag = newelem(STRING, inp-1, 1, 0);
			appendlist(fraglist, frag);
                    }
                    else { // DQTSTR|SQTSTR -- prop that start at next char - so new frag to skip this char
			frag = newelem(props & proptypemask, inp, 0, 0);
			appendlist(fraglist, frag);
                    }
                }
 		else { // new props don't match, so something got terminated and needs promoting
//printj(fraglist);
                    if (fraglist->props & (STRING|ESCAPE)) {
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

        if (props & (OPEN|CLOSE)) {
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
            if (props & (STRING|ESCAPE)) {
		fraglist->props = (STRING|ESCAPE);
		frag = newelem(STRING, inp-1, 1, 0);
		appendlist(fraglist, frag);
            }
            else if (props & (DQTSTR|SQTSTR)) {
		fraglist->props = (STRING|ESCAPE);
		frag = newelem(STRING, inp, 0, 0);
		appendlist(fraglist, frag);
            }
	    else if (props & SPACE) {
		fraglist->props = SPACE;
		frag = newelem(SPACE, inp-1, 1, 0);
		appendlist(fraglist, frag);
	    }
// FIXME - something for quoted strings
	}
    }
    return 0;
}
