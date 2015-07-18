#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "parse.h"
#include "list.h"

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
} state_t;

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
} props_t;

static int ACT_nxt[] =		{ACTION|OPT, SUBJECT, PROPERTIES|OPT, CONTAINER|OPT, TERM|OPT, NUL};
static int ACTION_nxt[] =	{TLD, NUL};
static int SUBJECT_nxt[] =	{OBJ|ALT, OBJLIST|ALT, NUL};
static int PROPERTIES_nxt[] =	{LBT, ATTR_VAL|REP|OPT, RBT, NUL};
static int CONTAINER_nxt[] =	{LBE, PROPERTIES|OPT, ACT|REP|OPT, RBE, NUL};
static int TERM_nxt[] =		{SCN|OPT, NUL};
static int OBJLIST_nxt[] =	{LPN, OBJ|REP|OPT, RPN, NUL};
static int OBJ_nxt[] =		{EDGE|ALT, NODE|ALT, NUL};
static int EDGE_nxt[] =		{LAN, LEG, LEG, LEG|REP|OPT, RAN, DISAMB|OPT, NUL};
static int LEG_nxt[] =		{ENDPOINT|ALT, ENDPOINTSET|ALT, NUL};
static int ENDPOINTSET_nxt[] =	{LPN, ENDPOINT|REP|OPT, RPN, NUL};
static int ENDPOINT_nxt[] =	{ANCESTOR|REP|OPT, DESCENDENT|REP|OPT, NODE, PORT|OPT, NUL};
static int DESCENDENT_nxt[] =	{NODEID, FSL, NUL};
static int PORT_nxt[] =		{CLN, PORTID, NUL};
static int NODELIST_nxt[] =	{LPN, NODE|REP|OPT, RPN, NUL};
static int NODE_nxt[] =		{NODEID, DISAMB|OPT, NUL};
static int DISAMB_nxt[] =	{DISAMBINTRO, DISAMBID, NUL};
static int ATTR_VAL_nxt[] =	{ATTRID, VALASSIGN|OPT, NUL};
static int VALASSIGN_nxt[] =	{EQL, VALUE, NUL};
static int NODEID_nxt[] =	{STRING|ALT, AST|ALT, NUL};
static int DISAMBID_nxt[] =	{STRING|ALT, AST|ALT, NUL};
static int PORTID_nxt[] =	{STRING|ALT, AST|ALT, NUL};
static int ATTRID_nxt[] =	{STRING, NUL};
static int VALUE_nxt[] =	{STRING, NUL};
static int DISAMBINTRO_nxt[] =	{CLN, CLN, NUL};
static int ANCESTOR_nxt[] =	{CLN, FSL, NUL};

static int *next[] = {
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

#if 1
props_t charONEstate2props[] = {
    /* NUL  */  NONE,
    /* ABC  */  STRING,
    /* WSP  */  SPACE,
    /*  LF  */  EOL | CMNTEOLEND | SPACE,
    /*  CR  */  TWO | EOL | CMNTEOLEND | SPACE,
    /* DQT  */  OPEN | CLOSE,
    /* SQT  */  OPEN | CLOSE,
    /* LPN  */  OPEN | PARENTHESIS,
    /* RPN  */  CLOSE | PARENTHESIS,
    /* LAN  */  OPEN | ANGLEBRACKET,
    /* RAN  */  OPEN | ANGLEBRACKET,
    /* LBR  */  OPEN | BRACKET,
    /* RBR  */  OPEN | BRACKET,
    /* LBE  */  OPEN | BRACE,
    /* RBE  */  OPEN | BRACE,
    /* FSL  */  TWO | CMNTBEG | CMNTEOLBEG,
    /* BSL  */  TWO | ESCAPE,
    /* OCT  */  NONE,
    /* AST  */  TWO | CMNTEND,
    /* CLN  */  NONE,
    /* SCN  */  NONE,
    /* EQL  */  NONE,
    /* TLD  */  NONE,
};
#else
int  charONEstate2props[];
charONEstate2props[NUL] =  NONE;
charONEstate2props[ABC] =  STRING;
charONEstate2props[WSP] =  SPACE;
charONEstate2props[ LF] =  EOL|CMNTEOLEND|SPACE;
charONEstate2props[ CR] =  TWO|EOL|CMNTEOLEND|SPACE;
charONEstate2props[DQT] =  OPEN|CLOSE;
charONEstate2props[SQT] =  OPEN|CLOSE;
charONEstate2props[LPN] =  OPEN|PARENTHESIS;
charONEstate2props[RPN] =  CLOSE|PARENTHESIS;
charONEstate2props[LAN] =  OPEN|ANGLEBRACKET;
charONEstate2props[RAN] =  OPEN|ANGLEBRACKET;
charONEstate2props[LBR] =  OPEN|BRACKET;
charONEstate2props[RBR] =  OPEN|BRACKET;
charONEstate2props[LBE] =  OPEN|BRACE;
charONEstate2props[RBE] =  OPEN|BRACE;
charONEstate2props[FSL] =  TWO|CMNTBEG|CMNTEOLBEG;
charONEstate2props[BSL] =  TWO|ESCAPE;
charONEstate2props[OCT] =  NONE;
charONEstate2props[AST] =  TWO|CMNTEND;
charONEstate2props[CLN] =  NONE;
charONEstate2props[SCN] =  NONE;
charONEstate2props[EQL] =  NONE;
charONEstate2props[TLD] =  NONE;
};
#endif

props_t charTWOstate2props[] = {
    /* NUL  */  NONE,
    /* ABC  */  ESCAPE, 
    /* WSP  */  ESCAPE,
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

int parse(unsigned char *inp) {
    unsigned char c;
    int rc, linecharcnt, linecnt, charsize;
    state_t state;
    props_t props;
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
        props = charONEstate2props[state];
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
