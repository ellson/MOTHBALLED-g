#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"

typedef enum {
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
 
// Every character is grouped into exactly one charclass_t
// All the characters in a charclass_t are treated identially by the parser
//
// charclass_t defaults to int, which would make this table 768 bytes larger
unsigned char char2charclass[] = {
   NUL, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* NUL SOH STX ETX EOT ENQ ACK BEL */
   ABC,  WS,  LF, ABC, ABC,  CR, ABC, ABC,  /*  BS TAB  LF  VT  FF  CR  SO  SI */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* DLE DC1 DC2 DC3 DC4 NAK STN ETB */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /* CAN  EM SUB ESC  FS  GS  RS  US */
    WS, ABC, DQT, OCT, ABC, ABC, ABC, SQT,  /* SPC  !   "   #   $   %   &   '  */
   LPN, RPN, AST, ABC, ABC, ABC, ABC, FSL,  /*  (   )   *   +   ,   -   .   /  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  0   1   2   3   4   5   6   7  */
   ABC, ABC, CLN, SCN, LAN, EQL, RAN, ABC,  /*  8   9   :   ;   <   =   >   ?  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  @   A   B   C   D   E   F   G  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  H   I   J   K   L   M   N   O  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  P   Q   R   S   T   U   V   W  */
   ABC, ABC, ABC, LBT, BSL, RBT, ABC, ABC,  /*  X   Y   Z   [   \   ]   ^   _  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  `   a   b   c   d   e   f   g  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  h   i   j   k   l   m   n   o  */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,  /*  p   q   r   s   t   u   v   w  */
   ABC, ABC, ABC, LBE, ABC, RBE, TLD, ABC,  /*  x   y   z   {   |   }   ~  DEL */
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
   ABC, ABC, ABC, ABC, ABC, ABC, ABC, ABC,
};
 
// each charclass can have zero or more properties, described by a bit pattern
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
   DISAMBIG     = 1<<18,
   ANCESTOR     = 1<<19,
   NPATH        = 1<<20,
   NLIST        = 1<<21,
} charprops_t;

// some of the property bits define the type of string fragment FIXME - may change as OPEN|CLOSE are dealt with ...
#define proptypemask (STRING | SPACE | ESCAPE | PARENTHESIS | ANGLEBRACKET | BRACKET | BRACE | DQTSTR | SQTSTR | CMNTSTR)

charprops_t charONEclass2props[] = {
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
    /* CLN  */  TWO | DISAMBIG  | ANCESTOR,
    /* SCN  */  NONE,
    /* EQL  */  NONE,
    /* TLD  */  NONE,
};

charprops_t charTWOclass2props[] = {
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
    /* CLN  */  ESCAPE | DISAMBIG,
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

static int opencloseblock(act_t *act, charclass_t charclass) {
    return 0;
}

unsigned char *test=(unsigned char*)"<a\\Aa 'bb' cc>";

int main (int argc, char *argv[]) {
    unsigned char *inp, c;
    int rc, linecharcnt, linecnt, charsize;
    charclass_t charclass;
    charprops_t charprops;
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

    inp = test;

    frag = NULL;
    linecharcnt = 0;
    linecnt = 0;
    while ((c = *inp++)) {
        linecharcnt++;
        charclass = char2charclass[c];
        charprops = charONEclass2props[charclass];
        charsize = 1;
        if (charprops & TWO) {
            if ((c = *inp)) {
	        charclass = char2charclass[c];

                // we work out what the character pair is by combining the 
                // charpros from both chars  (this means that CRLF is only a single EOL)
                // and then masking out the TWO and other unrelated bits
                charprops |= charTWOclass2props[charclass];
                charprops &= proptypemask;
                if (charprops) {
		    inp++;
                    charsize++;
	            if ((charprops & EOL)) {
                        linecnt++;
                        linecharcnt = 0;
                    }
		}
	    }
        }
        if (frag) {    // a frag already exists from a previous iteration of the while loop
            assert (frag->type == STR);
            if (charprops & frag->props) {  // matching charprops, just continue appending to frag
                frag->u.str.len += charsize;
            }
	    else {
                if (charprops & fraglist->props) {  // if matches fraglist 
 		    if (charprops & (STRING|SPACE)) {  // STRING or SPACE char that can be simply appended
                        frag->u.str.len += charsize;
                    }
                    else if (charprops & ESCAPE) {  // STRING frag that starts at this char - needs a new frag to skip BSL char
			frag = newelem(STRING, inp-1, 1, 0);
			appendlist(fraglist, frag);
                    }
                    else { // DQTSTR|SQTSTR -- charprop that start at next char - so new frag to skip this char
			frag = newelem(charprops & proptypemask, inp, 0, 0);
			appendlist(fraglist, frag);
                    }
                }
 		else { // new charprops don't match, so something got terminated and needs promoting
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

        if (charprops & (OPEN|CLOSE)) {
	    rc = opencloseblock(act, charclass);
	    if (rc) {
		fprintf(stderr, "parser error at: %d:%d \"%c\"\n", linecnt,  linecharcnt, c);
                exit(rc);
            }
        }
// FIXME - deal with OPEN and CLOSE
//     NPATH | NLIST --> EDGE
//     EDGE --> ELIST

        if (!frag) { // no current frag
            if (charprops & (STRING|ESCAPE)) {
		fraglist->props = (STRING|ESCAPE);
		frag = newelem(STRING, inp-1, 1, 0);
		appendlist(fraglist, frag);
            }
            else if (charprops & (DQTSTR|SQTSTR)) {
		fraglist->props = (STRING|ESCAPE);
		frag = newelem(STRING, inp, 0, 0);
		appendlist(fraglist, frag);
            }
	    else if (charprops & SPACE) {
		fraglist->props = SPACE;
		frag = newelem(SPACE, inp-1, 1, 0);
		appendlist(fraglist, frag);
	    }
// FIXME - something for quoted strings
	}
    }
}
